<?php

/**
 * @file
 * Components Group functions for administration purposes.
 * Specifically for the Display Overview.
 */

class ComponentsGroupAdminFormDisplayDecorator extends ComponentsGroupAdminFormDecorator {
  
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
      $view_mode = $this->view_mode;
      
      $table[$name]['#region_callback'] = 'components_group_display_overview_row_region';
      
      $default_weight = 0;
      if (isset($group_data['display'][$view_mode]['weight'])) {
        $default_weight = $group_data['display'][$view_mode]['weight'];
      }
      $table[$name]['weight']['#default_value'] = $default_weight;
      
      $default_visible = 0;
      if (isset($group_data['display'][$view_mode]['visible'])) {
        $default_visible = $group_data['display'][$view_mode]['visible'];
      }
      $table[$name]['format'] = array(
        'visible' => array(
          '#type' => 'select',
          '#options' => array(1 => t('Visible'), 0 => t('Hidden')),
          '#default_value' => $default_visible,
        ),
      );
    }
    
    $form['#submit'][] = 'components_group_display_overview_submit';
  }
  
}

/**
 * Implements hook_form_FORM_ID_alter().
 * Using hook_form_field_ui_display_overview_form_alter().
 */
function components_group_form_field_ui_display_overview_form_alter(&$form, &$form_state) {
  // Only alter the form if needed
  if (empty($form['#fields']) && empty($form['#extra'])) {
    return;
  }
  
  $admin_form = new ComponentsGroupAdminFormDefault($form, $form_state);
  $admin_form_decorated = new ComponentsGroupAdminFormDisplayDecorator($admin_form);
}

function components_group_display_overview_row_region($row) {
  switch ($row['#row_type']) {
    case 'components_group':
      return ($row['format']['visible']['#value'] == 1) ? 'visible' : 'hidden';
      break;
  }
}

/**
 * Submit handler for the display overview form.
 */
function components_group_display_overview_submit($form, &$form_state) {
  $form_values = $form_state['values'];
  $entity_type = $form['#entity_type'];
  $bundle = $form['#bundle'];
  $view_mode = $form['#view_mode'];
  
  foreach ($form['#components_group_ids'] as $cgid) {
    if ($components_group = components_group_load($cgid)) {
      $name = $components_group->identifier();
      if (isset($form_values['fields'][$name])) {
        $values = $form_values['fields'][$name];
        $settings = &$components_group->data;
        $settings['display'][$view_mode] = array(
          'weight' => $values['weight'],
          'visible' => $values['format']['visible'],
        );
        components_group_save($components_group);
      }
    }
  }
}
