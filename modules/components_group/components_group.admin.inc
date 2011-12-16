<?php

/**
 * @file
 * Components Group function for administration purposes.
 */

class ComponentsGroupFormParams {
  
  public $entity_type;
  public $bundle;
  public $region_callback;
  public $groups;
  
  public function __construct($form) {
    $this->entity_type = $form['#entity_type'];
    $this->bundle = $form['#bundle'];
    $this->region_callback = 'components_group_field_overview_row_region';
    $this->setGroups();
  }
  
  public function setGroups() {
    $this->groups = components_group_load_by_entity_type_bundle($this->entity_type, $this->bundle);
  }
  
}

function components_group_field_ui_form_params($form) {
  return new ComponentsGroupFormParams($form);
}

/**
 * Return an array with all "entity type - bundle" combinations which are using
 * components groups.
 * 
 * @return array $return
 *   An array in the form of:
 *     Array (
 *       [node] => Array (
 *         [0] => article
 *         [1] => page
 *       )
 *       [custom_entity] => Array (
 *         [0] => custom_bundle
 *       )
 *     )
 */
function components_group_get_host_entity_types() {
  $return = array();
  // @TODO: cache results
  $query = db_select('components_group', 'cg');
  $query->fields('cg', array('entity_type', 'bundle'));
  $query->condition('cg.status', 1, '=');
  $query->distinct();
  $results = $query->execute();
  
  foreach ($results as $result) {
    $entity_type = $result->entity_type;
    $bundle = $result->bundle;
    if (isset($return[$entity_type])) {
      $return[$entity_type] = array_merge($return[$entity_type], array($bundle));
    }
    else {
      $return[$entity_type] = array($bundle);
    }
  }
  
  return $return;
}

/**
 * Checks wether the given combination of "entity type - bundle" is
 * using components groups. Returns TRUE if so, otherwise FALSE.
 * 
 * @param $entity_type
 *   The entity type of an entity (e.g. "node",...).
 * @param $bundle
 *   The bundle of an entity (e.g. "article", "page",...).
 * @return boolean $result
 *   TRUE or FALSE, depending of the combination
 *   "entity type - bundle" uses components groups
 */
function components_group_entity_type_uses_components_groups($entity_type, $bundle) {
  $result = FALSE;
  $entity_types = components_group_get_host_entity_types();
  if (isset($entity_types[$entity_type])) {
    if (in_array($bundle, $entity_types[$entity_type])) {
      $result = TRUE;
    }
  }
  return $result;
}

/**
 * Implements hook_field_attach_view_alter().
 */
function components_group_field_attach_view_alter(&$element, $context) {
  if (isset($element['#entity_type']) && isset($element['#bundle'])) {
    if (components_group_entity_type_uses_components_groups($element['#entity_type'], $element['#bundle'])) {
      components_group_attach_components_groups($element, $context);
    }
  }
}

function components_group_attach_components_groups(&$element, $context) {
  $entity_type = $element['#entity_type'];
  $bundle = $element['#bundle'];
  
  $element['#components_groups_context'] = $context;
  $element['#components_groups'] = components_group_load_by_entity_type_bundle($entity_type, $bundle);
  
  $element['#pre_render'][] = 'components_group_pre_render';
}

function components_group_pre_render($element) {
  if (empty($element['#components_groups_context']) || empty($element['#components_groups'])) {
    return $element;
  }
  
  $context = $element['#components_groups_context'];
  list($entity_id) = entity_extract_ids($context['entity_type'], $context['entity']);
  
  foreach ($element['#components_groups'] as $components_group) {
    $element[$components_group->identifier()] = array(
      '#components' => component_load_all_by_host(
        $context['entity_type'],
        $entity_id,
        $components_group->internalIdentifier()
      ),
    ) + components_group_view($components_group);
  }
  
  return $element;
}

/**
 * Implements hook_form_FORM_ID_alter().
 * Using hook_form_field_ui_field_overview_form_alter().
 */
function components_group_form_field_ui_field_overview_form_alter(&$form, &$form_state) {
  components_group_form_field_ui_overview_form_alter($form, $form_state);
}

/**
 * Implements hook_form_FORM_ID_alter().
 * Using hook_form_field_ui_display_overview_form_alter().
 */
function components_group_form_field_ui_display_overview_form_alter(&$form, &$form_state) {
  components_group_form_field_ui_overview_form_alter($form, $form_state, TRUE);
}

/**
 * Function to alter the field overview and display overview forms.
 */
function components_group_form_field_ui_overview_form_alter(&$form, &$form_state, $display_overview = FALSE) {
  // Only alter the form if needed
  if (empty($form['#fields']) && empty($form['#extra'])) {
    return;
  }
  
  $params = components_group_field_ui_form_params($form);
  
  $form['#components_group_ids'] = array();
  foreach ($params->groups as $group) {
    $form['#components_group_ids'][] = $group->cgid;
  }
  
  $table = &$form['fields'];
  
  // Add a region for 'add_new_components_group' rows, but only when fields are
  // available and thus regions.
  /*if (isset($table['#regions'])) {
    $table['#regions'] += array(
      'add_new_components_group' => array('title' => '&nbsp;'),
    );
  }
  
  // Extend available parenting options.
  /*foreach ($params->groups as $group) {
    $table['#parent_options'][$group->identifier()] = $group->label;
  }
  $table['#parent_options']['_add_new_components_group'] = t('Add new components group');
  
  // Update existing rows accordingly to the parents.
  foreach (element_children($table) as $name) {
    $table[$name]['parent_wrapper']['parent']['#options'] = $table['#parent_options'];
    // Inherit the value of the parent when default value is empty.
    if (empty($table[$name]['parent_wrapper']['parent']['#default_value'])) {
      $table[$name]['parent_wrapper']['parent']['#default_value'] = isset($params->parents[$name]) ? $params->parents[$name] : '';
    }
  }*/
  
  // Create the components group rows
  foreach ($params->groups as $group) {
    $name = $group->identifier();
    
    $form_state['components_group'][$name] = $group;
    
    $html_id = drupal_html_id($name);
    
    $parent_options = $table['#parent_options'];
    unset($parent_options[$name]);
    
    $table[$name] = array(
      '#attributes' => array('class' => array('draggable', 'tabledrag-leaf', 'field-components-group'), 'id' => $html_id),
      '#row_type' => 'components_group',
      '#region_callback' => $params->region_callback,
      'label' => array(
        '#markup' => check_plain($group->label),
      ),
      'weight' => array(
        '#type' => 'textfield',
        '#default_value' => $group->weight,
        '#size' => 3,
        '#attributes' => array('class' => array('field-weight')),
      ),
      'parent_wrapper' => array(
        'parent' => array(
          '#type' => 'select',
          '#options' =>  $parent_options,
          '#empty_value' => '',
          '#default_value' => isset($params->parents[$name]) ? $params->parents[$name] : '',
          '#attributes' => array('class' => array('field-parent')),
          '#parents' => array('fields', $name, 'parent'),
        ),
        'hidden_name' => array(
          '#type' => 'hidden',
          '#default_value' => $name,
          '#attributes' => array('class' => array('field-name')),
        ),
      ),
      'components_group_name' => array(
        '#markup' => check_plain($name),
      ),
      'type' => array(
        '#markup' => t('Components Group'),
      ),
      'widget_type' => array(),
      'edit' => array(
        '#type' => 'link',
        '#title' => t('edit'),
        '#href' => '',
        '#options' => array('attributes' => array('title' => t('Edit instance settings.'))),
      ),
      'delete' => array(
        '#type' => 'link',
        '#title' => t('delete'),
        '#href' => '',
        '#options' => array('attributes' => array('title' => t('Delete instance.'))),
      ),
    );
  }
  
  if (!$display_overview) {
    // Additional row: add new components group
    $max_weight = field_info_max_weight($params->entity_type, $params->bundle, 'form');
    $parent_options = $table['#parent_options'];
    //unset($parent_options['_add_new_components_group']);
    
    $name = '_add_new_components_group';
    $table[$name] = array(
      '#attributes' => array('class' => array('draggable', 'components-group', 'add-new')),
      '#row_type' => 'add_new_components_group',
      '#region_callback' => $params->region_callback,
      'label' => array(
        '#type' => 'textfield',
        '#title' => t('New component group label'),
        '#title_display' => 'invisible',
        '#size' => 15,
        '#description' => t('Label'),
        '#prefix' => '<div class="label-input"><div class="add-new-placeholder">' . t('Add new components group') . '</div>',
        '#suffix' => '</div>',
      ),
      'weight' => array(
        '#type' => 'textfield',
        '#title' => t('Weight for component group'),
        '#title_display' => 'invisible',
        '#default_value' => $max_weight + 1,
        '#size' => 3,
        '#attributes' => array('class' => array('field-weight')),
        '#prefix' => '<div class="add-new-placeholder">&nbsp;</div>',
      ),
      'parent_wrapper' => array(
        'parent' => array(
          '#type' => 'select',
          '#title' => t('Parent for new component group'),
          '#title_display' => 'invisible',
          '#options' => $table['#parent_options'],
          '#empty_value' => '',
          '#attributes' => array('class' => array('field-parent')),
          '#prefix' => '<div class="add-new-placeholder">&nbsp;</div>',
          '#parents' => array('fields', $name, 'parent'),
        ),
        'hidden_name' => array(
          '#type' => 'hidden',
          '#default_value' => $name,
          '#attributes' => array('class' => array('field-name')),
        ),
      ),
      'components_group_name' => array(
        /*'#type' => 'textfield',
        '#title' => t('New component group name'),
        '#title_display' => 'invisible',
        // This field should stay LTR even for RTL languages.
        '#field_prefix' => '<span dir="ltr">components_group_',
        '#field_suffix' => '</span>&lrm;',
        '#attributes' => array('dir' => 'ltr'),
        '#size' => 10,
        '#description' => t('Components group name (a-z, 0-9, _)'),
        '#prefix' => '<div class="add-new-placeholder">&nbsp;</div>',*/
        '#cell_attributes' => array('colspan' => 2),
      ),
      'format' => array(),
      'settings_summary' => array(),
      'settings_edit' => array(),
    );
  }
  
  $path = drupal_get_path('module', 'components_group');
  $form['#attached']['css'][] = $path . '/components-group.field-ui.css';
  
  $form['#validate'][] = 'components_group_field_overview_validate';
  $form['#submit'][] = 'components_group_field_overview_submit';
}

function components_group_field_overview_row_region($row) {
  switch ($row['#row_type']) {
    case 'components_group':
      return 'main';
    case 'add_new_components_group':
      // If no input in 'label', assume the row has not been dragged out of the
      // 'add new' section.
      return (!empty($row['label']['#value']) ? 'main' : 'add_new');
  }
}

function components_group_field_overview_validate($form, &$form_state) {
  if ($form_state['values']['fields']['_add_new_components_group']) {
    $group = $form_state['values']['fields']['_add_new_components_group'];
    // Validate if any information was provided in the 'add new components group' row
    if (array_filter(array($group['label']))) {
      // Missing label
      if (!$group['label']) {
        form_set_error('fields][_add_new_components_group][label', t('Add new components group: you need to provide a label.'));
      }
    }
  }
}

function components_group_field_overview_submit($form, &$form_state) {
  $form_values = $form_state['values']['fields'];
  $entity_type = $form['#entity_type'];
  $bundle = $form['#bundle'];
  
  // Save new components group
  if (!empty($form_values['_add_new_components_group']['label'])) {
    $values = $form_values['_add_new_components_group'];
    
    $components_group = components_group_create(array(
      'entity_type' => $entity_type,
      'bundle' => $bundle,
      'label' => $values['label'],
      'data' => '',
    ));
    
    if (components_group_save($components_group)) {
      drupal_set_message(t('New components group %label successfully created.', array('%label' => $components_group->label)));
    }
    else {
      drupal_set_message(t('Creating new components group %label failed.', array('%label' => $values['label'])), 'error');
    }
  }
  
  // Update existing components groups
  foreach ($form['#components_group_ids'] as $cgid) {
    if ($components_group = components_group_load($cgid)) {
      $name = $components_group->identifier();
      $components_group->entity_type = $entity_type;
      $components_group->bundle = $bundle;
      components_group_save($components_group);
    }
  }
}
