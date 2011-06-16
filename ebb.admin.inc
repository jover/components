<?php

/**
 * @file
 * EntityBuildingBlock editing UI.
 */

/**
 * UI Controller
 */
class EntityBuildingBlockTypeUIController extends EntityDefaultUIController {
  
  /**
   * Overrides hook_menu() defaults.
   */
  public function hook_menu() {
    $items = parent::hook_menu();
    $items[$this->path]['description'] = t('Manage Entity Building Blocks, including fields.');
    return $items;
  }
  
}

/**
 * Generates the Entity Building Block Type editing form.
 */
function ebb_type_form($form, &$form_state, $ebb_type, $op = 'edit') {
  if ($op == 'clone') {
    $ebb_type->label .= ' (cloned)';
    //$ebb_type->type = '';
    $ebb_type->type .= '_clone';
  }
  
  $form['label'] = array(
    '#title' => t('Label'),
    '#type' => 'textfield',
    '#default_value' => $ebb_type->label,
    '#description' => t('The human-readable name of this type.'),
    '#required' => TRUE,
    '#size' => 30,
    '#weight' => 0,
  );
  
  // Machine-readable type name.
  $form['type'] = array(
    '#type' => 'machine_name',
    '#default_value' => isset($ebb_type->type) ? $ebb_type->type : '',
    '#maxlength' => 32,
    '#disabled' => $ebb_type->isLocked() && $op != 'clone',
    '#machine_name' => array(
      'exists' => 'ebb_get_types',
      'source' => array('label'),
    ),
    '#description' => t('A unique machine-readable name for this Entity Building Block Type. It must only contain lowercase letters, numbers, and underscores.'),
    '#weight' => 1,
  );
  
  $form['description'] = array(
    '#title' => t('Description'),
    '#type' => 'textarea',
    '#default_value' => $ebb_type->description,
    '#description' => t('A brief description of this type.'),
    '#required' => TRUE,
    '#rows' => 2,
    '#weight' => 3,
  );
  
  $form['help'] = array(
    '#title' => t('Help'),
    '#type' => 'textarea',
    '#default_value' => $ebb_type->help,
    '#description' => t('Help information shown to the user when creating an Entity Building Block of this type.'),
    '#required' => FALSE,
    '#rows' => 2,
    '#weight' => 4,
  );
  
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#weight' => 5,
  );
  
  if (!$ebb_type->isLocked() && $op != 'add') {
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#weight' => 45,
      '#limit_validation_errors' => array(),
      '#submit' => array('ebb_type_form_submit_delete'),
    );
  }
  
  field_attach_form('ebb_type', $ebb_type, $form, $form_state);
  
  return $form;
}

/**
 * Form API submit callback for the type form.
 */
function ebb_type_form_submit(&$form, &$form_state) {
  $ebb_type = entity_ui_form_submit_build_entity($form, $form_state);
  // Save and go back.
  $ebb_type->save();
  $form_state['redirect'] = 'admin/structure/entity-building-block-types';
}

/**
 * Form API submit callback for the delete button.
 */
function ebb_type_form_submit_delete(&$form, &$form_state) {
  $form_state['redirect'] = 'admin/structure/entity-building-block-types/manage/' . $form_state['ebb_type']->type . '/delete';
}

/**
 * Implements hook_form_FORM_ID_alter().
 */
function ebb_form_ebb_type_form_alter(&$form, &$form_state) {
  if (isset($form['field_ebb_type_image'])) {
    $form['field_ebb_type_image']['#weight'] = 3;
  }
}
