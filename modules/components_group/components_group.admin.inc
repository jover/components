<?php

/**
 * @file
 * Components Group functions for administration purposes.
 */

abstract class ComponentsGroupAdminForm {
  
  public $form;
  public $form_state;
  protected $entity_type;
  protected $bundle;
  protected $components_groups;
  
  protected abstract function decorateForm();
  
}

class ComponentsGroupAdminFormDefault extends ComponentsGroupAdminForm {
  
  public function __construct($form, $form_state) {
    $this->form = $form;
    $this->form_state = $form_state;
    $this->setFormParams();
    $this->setComponentsGroups();
    $this->decorateForm();
  }
  
  protected function setFormParams() {
    $this->entity_type = $this->form['#entity_type'];
    $this->bundle = $this->form['#bundle'];
  }
  
  protected function setComponentsGroups() {
    $this->components_groups = components_group_load_by_entity_type_bundle($this->entity_type, $this->bundle);
  }
  
  protected function decorateForm() {
    $form = &$this->form;
    $form_state = &$this->form_state;
    $table = &$form['fields'];
    
    $form['#components_group_ids'] = array();
    foreach ($this->components_groups as $group) {
      // Store all components group IDs
      $form['#components_group_ids'][] = $group->identifier();
      
      // Create the components group rows
      $name = $group->name();
      
      $form_state['components_group'][$name] = $group;
      
      $html_id = drupal_html_id($name);
      
      $parent_options = $table['#parent_options'];
      unset($parent_options[$name]);
      
      $table[$name] = array(
        '#attributes' => array('class' => array('draggable', 'tabledrag-leaf', 'field-components-group'), 'id' => $html_id),
        '#row_type' => 'components_group',
        '#region_callback' => NULL, // must be overriden by decorator
        '#js_settings' => array('rowHandler' => 'components_group'),
        'label' => array(
          '#markup' => check_plain($group->label),
        ),
        'weight' => array(
          '#type' => 'textfield',
          '#default_value' => 0, // must be overriden by decorator
          '#size' => 3,
          '#attributes' => array('class' => array('field-weight')),
        ),
        'parent_wrapper' => array(
          'parent' => array(
            '#type' => 'select',
            '#options' =>  $parent_options,
            '#empty_value' => '',
            //'#default_value' => isset($params->parents[$name]) ? $params->parents[$name] : '',
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
        'format' => array(),
        'widget_type' => array(),
        'edit' => array(),
      );
      
      $path = drupal_get_path('module', 'components_group');
      $form['#attached']['css'][] = $path . '/components-group.field-ui.css';
    }
  }
  
}

abstract class ComponentsGroupAdminFormDecorator extends ComponentsGroupAdminForm {
  
  protected $admin_form;
  
  protected function setFormParams() {
    $this->form = $this->admin_form->form;
    $this->form_state = $this->admin_form->form_state;
    $this->entity_type = $this->admin_form->entity_type;
    $this->bundle = $this->admin_form->bundle;
    $this->components_groups = $this->admin_form->components_groups;
  }
  
}

module_load_include('inc', 'components_group', 'components_group.admin.display');
module_load_include('inc', 'components_group', 'components_group.admin.field');

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
    $element[$components_group->name()] = components_group_view(
      $components_group,
      $context['entity_type'],
      $entity_id,
      $context['view_mode']
    );
  }
  
  return $element;
}

/**
 * Menu callback: returns a form for removing a components group.
 */
function components_group_delete_form($form, &$form_state, $components_group) {
  $admin_path = _field_ui_bundle_admin_path($components_group->entity_type, $components_group->bundle);
  $admin_path .= '/fields';
  
  $form['cgid'] = array(
    '#type' => 'hidden',
    '#value' => $components_group->identifier(),
  );
  
  $form['redirect'] = array(
    '#type' => 'hidden',
    '#value' => $admin_path,
  );
  
  return confirm_form(
    $form,
    t('Are you sure you want to delete the components group %label?', array('%label' => $components_group->label)),
    $admin_path,
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel'),
    'confirm'
  );
}

function components_group_delete_form_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    $components_group = components_group_load($form_state['values']['cgid']);
    components_group_delete($components_group->identifier());
    watchdog('components_group', 'Components Group: deleted cgid=@cgid', array('@cgid' => $components_group->identifier()));
    drupal_set_message(t('The components group (%label) has been deleted.', array('%label' => $components_group->label)));
  }
  $form_state['redirect'] = $form_state['values']['redirect'];
}

/**
 * Menu callback: returns a form for editing a components group.
 */
function components_group_edit_form($form, &$form_state, $components_group) {
  $entity_type = $components_group->entity_type;
  $bundle = $components_group->bundle;
  $bundles = field_info_bundles();
  $bundle_label = $bundles[$entity_type][$bundle]['label'];
  
  $settings = $components_group->data;
  $settings_component_layouts = (isset($settings['component_layouts'])) ? $settings['component_layouts'] : array();
  
  drupal_set_title($components_group->label);
  
  $form['#components_group'] = $components_group;
  
  $form['components_group'] = array(
    '#tree' => TRUE,
    '#type' => 'fieldset',
    '#title' => t('%label components group settings', array('%label' => $components_group->label)),
    '#description' => t('These settings apply to the %label components group that is part of the %type type.', array(
      '%label' => $components_group->label,
      '%type' => $bundle_label,
    )),
  );
  
  $form['components_group']['label'] = array(
    '#type' => 'textfield',
    '#title' => t('Label'),
    '#default_value' => $components_group->label,
    '#required' => TRUE,
    '#weight' => -20,
  );
  
  $form['components_group']['allowed_component_layouts'] = array(
    '#tree' => TRUE,
    '#type' => 'fieldset',
    '#title' => t('Allowed component layouts'),
    '#description' => t('Check the component layouts which may be added to this components group.'),
  );
  
  // @TODO: refactor code below: similar code in component_wizard_add_component_type_layout()
  $layouts = components_get_layouts();
  
  $bundles = array();
  foreach ($layouts as $id => $layout) {
    if (isset($layout->bundle)) {
      $bundle = $layout->bundle;
      $bundles[$bundle][$id] = $layout;
    }
  }
  
  $form['components_group']['allowed_component_layouts']['component_types'] = array(
    '#type' => 'vertical_tabs',
  );
  
  foreach ($bundles as $bundle => $bundle_layouts) {
    $bundle_label = component_type_get_label_by_type($bundle);
    
    $form['components_group']['allowed_component_layouts'][$bundle] = array(
      '#type' => 'fieldset',
      '#title' => ($bundle_label) ? $bundle_label : $bundle,
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#group' => 'components_group][allowed_component_layouts][component_types',
    );
    
    foreach ($bundle_layouts as $id => $layout) {
      if (!is_null($layout->image) && $image_file = file_load($layout->image)) {
        $image = theme('image_style', array(
          'path' => $image_file->uri,
          'style_name' => 'thumbnail',// @TODO: create custom image style
          'alt' => $layout->label,
          'title' => $layout->label,
        ));
      }
      else { // Default image
        $image = theme('image', array(
          'path' => drupal_get_path('module', 'components') . '/images/components.png',
          'alt' => $layout->label,
          'title' => $layout->label,
          'height' => 100, // @TODO: change to size of image style
          'width' => 100,
        ));
      }
      
      $form['components_group']['allowed_component_layouts'][$bundle][$id] = array(
        '#type' => 'checkbox',
        '#title' => $image . '<span>' . $layout->label . '</span>',
        '#return_value' => TRUE,
        '#default_value' => (in_array($id, $settings_component_layouts)) ? TRUE : FALSE,
        '#parents' => array('components_group', 'component_layouts', $id),
      );
    }
  }
  
  $form['actions'] = array(
    '#type' => 'actions',
  );
  
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save settings'),
  );
  
  return $form;
}

function components_group_edit_form_validate($form, &$form_state) {
  
}

function components_group_edit_form_submit($form, &$form_state) {
  $form_values = $form_state['values'];
  $values = $form_values['components_group'];
  
  $components_group = $form['#components_group'];
  
  $components_group->label = $values['label'];
  
  $component_layouts = array();
  foreach ($values['component_layouts'] as $layout => $value) {
    if ($value) { // if value is 1 (checked = TRUE), thus not 0 (checked = FALSE)
      $component_layouts[] = $layout;
    }
  }
  $components_group->data['component_layouts'] = $component_layouts;
  
  components_group_save($components_group);
  drupal_set_message(t('Saved %label components group settings.', array('%label' => $components_group->label)));
  
  $redirect = _field_ui_bundle_admin_path($components_group->entity_type, $components_group->bundle);
  $form_state['redirect'] = $redirect . '/fields';
}
