<?php

/**
 * @file
 * Components Group functions for administration purposes.
 * Specifically for the Field Overview.
 */

class ComponentsGroupAdminFormFieldDecorator extends ComponentsGroupAdminFormDecorator {
  
  public function __construct(ComponentsGroupAdminForm $admin_form) {
    $this->admin_form = $admin_form;
    $this->setFormParams();
    $this->decorateForm();
  }
  
  protected function decorateForm() {
    $form = &$this->form;
    $form_state = &$this->form_state;
    $table = &$form['fields'];
    
    foreach ($this->components_groups as $group) {
      $name = $group->identifier();
      $group_data = $group->data;
      
      $table[$name]['#region_callback'] = 'components_group_field_overview_row_region';
      
      $table[$name]['format'] = array(
        '#markup' => t('Components Group'),
      );
      $table[$name]['delete'] = array(
        '#type' => 'link',
        '#title' => t('delete'),
        '#href' => 'components-group/' . $group->internalIdentifier() . '/delete',
        '#options' => array('attributes' => array('title' => t('Delete instance.'))),
      );
    }
    
    $this->decorateFormAddNewRow();
    
    $form['#validate'][] = 'components_group_field_overview_validate';
    $form['#submit'][] = 'components_group_field_overview_submit';
  }
  
  protected function decorateFormAddNewRow() {
    $form = &$this->form;
    $form_state = &$this->form_state;
    $table = &$form['fields'];
    
    // Additional row: add new components group
    $max_weight = field_info_max_weight($this->entity_type, $this->bundle, 'form');
    $parent_options = $table['#parent_options'];
    //unset($parent_options['_add_new_components_group']);
    
    $name = '_add_new_components_group';
    $table[$name] = array(
      '#attributes' => array('class' => array('draggable', 'components-group', 'add-new')),
      '#row_type' => 'add_new_components_group',
      '#region_callback' => 'components_group_field_overview_row_region',
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
  
}

/**
 * Implements hook_form_FORM_ID_alter().
 * Using hook_form_field_ui_field_overview_form_alter().
 */
function components_group_form_field_ui_field_overview_form_alter(&$form, &$form_state) {
  // Only alter the form if needed
  if (empty($form['#fields']) && empty($form['#extra'])) {
    return;
  }
  
  $admin_form = new ComponentsGroupAdminFormDefault($form, $form_state);
  $admin_form_decorated = new ComponentsGroupAdminFormFieldDecorator($admin_form);
  
  $form = $admin_form_decorated->form;
  $form_state = $admin_form_decorated->form_state;
}

function components_group_field_overview_row_region($row) {
  switch ($row['#row_type']) {
    case 'components_group':
      return 'main';
      break;
    case 'add_new_components_group':
      // If no input in 'label', assume the row has not been dragged out of the
      // 'add new' section.
      return (!empty($row['label']['#value']) ? 'main' : 'add_new');
      break;
  }
}

/**
 * Validate handler for the field overview form.
 */
function components_group_field_overview_validate($form, &$form_state) {
  if (isset($form_state['values']['fields']['_add_new_components_group'])) {
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

/**
 * Submit handler for the field overview form.
 */
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
      'data' => array(),
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
