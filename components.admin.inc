<?php

/**
 * @file
 * Component editing UI.
 */

/**
 * UI Controller
 */
class ComponentTypeUIController extends EntityDefaultUIController {
  
  /**
   * Overrides hook_menu() defaults.
   */
  public function hook_menu() {
    $items = parent::hook_menu();
    $items[$this->path]['description'] = t('Manage components, including fields.');
    return $items;
  }
  
/**
   * Generates the render array for a overview table for arbitrary entities
   * matching the given conditions.
   *
   * @param $conditions
   *   An array of conditions as needed by entity_load().

   * @return Array
   *   A renderable array.
   */
  public function overviewTable($conditions = array()) {
    $table = parent::overviewTable($conditions);
    $header_items = array();
    foreach ($table['#header'] as $delta => $header) {
      // insert extra header items before the original second item
      // @TODO: wait for drupal_array_insert() : http://drupal.org/node/66183
      if ($delta == 1) {
        $header_items[] = t('Description');
        $header_items[] = t('Sample Image');
      }
      $header_items[] = $header;
    }
    $table['#header'] = $header_items;
    return $table;
  }
  
/**
   * Generates the row for the passed entity and may be overridden in order to
   * customize the rows.
   *
   * @param $additional_cols
   *   Additional columns to be added after the entity label column.
   */
  protected function overviewTableRow($conditions, $id, $entity, $additional_cols = array()) {
    // Add description column
    $additional_cols[] = (isset($entity->description)) ? $entity->description : t('No description');
    
    // Add sample image column
    if (isset($entity->field_component_type_image) && $image = field_get_items('component_type', $entity, 'field_component_type_image')) {
      $image = reset($image);
      $image_vars = array(
        'style_name' => 'thumbnail', // @TODO: create custom image styles for sample images (component_type_sample_image & component_type_sample_image_thumbnail)
        'path' => $image['uri'],
        'alt' => t('@label sample image', array('@label' => $entity->label)),
      	'title' => t('@title sample image', array('@title' => $entity->label)),
      );
      $additional_cols[] = theme_image_style($image_vars);
    }
    else {
      $additional_cols[] = t('No sample image');
    }
    
    return parent::overviewTableRow($conditions, $id, $entity, $additional_cols);
  }
  
}

/**
 * Generates the component type editing form.
 */
function component_type_form($form, &$form_state, $component_type, $op = 'edit') {
  if ($op == 'clone') {
    $component_type->label .= ' (cloned)';
    //$component_type->type = '';
    $component_type->type .= '_clone';
  }
  
  $form['label'] = array(
    '#title' => t('Label'),
    '#type' => 'textfield',
    '#default_value' => $component_type->label,
    '#description' => t('The human-readable name of this type.'),
    '#required' => TRUE,
    '#size' => 30,
    '#weight' => 0,
  );
  
  // Machine-readable type name.
  $form['type'] = array(
    '#type' => 'machine_name',
    '#default_value' => isset($component_type->type) ? $component_type->type : '',
    '#maxlength' => 32,
    '#disabled' => $component_type->isLocked() && $op != 'clone',
    '#machine_name' => array(
      'exists' => 'components_get_types',
      'source' => array('label'),
    ),
    '#description' => t('A unique machine-readable name for this component type. It must only contain lowercase letters, numbers, and underscores.'),
    '#weight' => 1,
  );
  
  $form['description'] = array(
    '#title' => t('Description'),
    '#type' => 'textarea',
    '#default_value' => $component_type->description,
    '#description' => t('A brief description of this type.'),
    '#required' => TRUE,
    '#rows' => 2,
    '#weight' => 3,
  );
  
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#weight' => 4,
  );
  
  if (!$component_type->isLocked() && $op != 'add') {
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#weight' => 45,
      '#limit_validation_errors' => array(),
      '#submit' => array('component_type_form_submit_delete'),
    );
  }
  
  field_attach_form('component_type', $component_type, $form, $form_state);
  
  return $form;
}

/**
 * Form API submit callback for the type form.
 */
function component_type_form_submit(&$form, &$form_state) {
  $component_type = entity_ui_form_submit_build_entity($form, $form_state);
  // Save and go back.
  $component_type->save();
  $form_state['redirect'] = 'admin/structure/component-types';
}

/**
 * Form API submit callback for the delete button.
 */
function component_type_form_submit_delete(&$form, &$form_state) {
  $form_state['redirect'] = 'admin/structure/component-types/manage/' . $form_state['component_type']->type . '/delete';
}

/**
 * Implements hook_form_FORM_ID_alter().
 */
function component_form_component_type_form_alter(&$form, &$form_state) {
  if (isset($form['field_component_type_image'])) {
    $form['field_component_type_image']['#weight'] = 3;
  }
}
