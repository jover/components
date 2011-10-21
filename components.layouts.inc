<?php

/**
 * @file
 * Components layout functionality
 */

/**
 * Implements hook_form_FORM_ID_alter().
 */
function components_form_field_ui_display_overview_form_alter(&$form, &$form_state) {
  if (isset($form['#entity_type']) && $form['#entity_type'] == 'component') {
    components_layout_settings_vertical_tabs($form);
    components_layout_settings($form, $form_state);
  }
}

function components_get_layouts() {
  return ctools_export_crud_load_all('components_layout');
}

function components_layout_settings_vertical_tabs(&$form) {
  // Add additional settings vertical tab if not present.
  if (!isset($form['additional_settings'])) {
    $form['additional_settings'] = array(
      '#type' => 'vertical_tabs',
      '#theme_wrappers' => array('vertical_tabs'),
      '#prefix' => '<div>',
      '#suffix' => '</div>',
      '#tree' => TRUE,
    );
    
    $form['#attached']['js'][] = 'misc/form.js';
    $form['#attached']['js'][] = 'misc/collapse.js';
  }
}

function components_layout_settings_export_id($bundle, $view_mode) {
  if (is_null($bundle) || is_null($view_mode)) {
    return FALSE;
  }
  return $bundle . '|' . $view_mode;
}

function components_layout_settings_get_view_mode_label($view_mode) {
  $entity_info = entity_get_info('component');
  $view_modes = $entity_info['view modes'];
  if ($view_mode == 'default') {
    return drupal_ucfirst($view_mode);
  }
  if (isset($view_modes[$view_mode])) {
    return $view_modes[$view_mode]['label'];
  }
  return FALSE;
}

function components_layout_settings(&$form, &$form_state) {
  ctools_include('export');
  
  $bundle = $form['#bundle'];
  $view_mode = $form['#view_mode'];
  
  $export_id = components_layout_settings_export_id($bundle, $view_mode);
  if ($export_id == FALSE) {
    return;
  }
  
  $layout = ctools_export_crud_load('components_layout', $export_id);
  
  if (!$layout) {
    $layout = ctools_export_crud_new('components_layout');
    $layout->id = $export_id;
    $layout->bundle = $bundle;
    $layout->view_mode = $view_mode;
    $layout->label = components_layout_settings_get_view_mode_label($view_mode);
    $layout->image = NULL;
  }
  
  $form['#components_layout'] = $layout;
  
  // Add component type specific additional settings
  $form['additional_settings']['components_layout_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Component layout settings'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#parents' => array('additional_settings'),
  );
  
  $form['additional_settings']['components_layout_settings']['component_layout_label'] = array(
    '#type' => 'textfield',
    '#title' => t('Layout label'),
    '#description' => t('An alias for the component type in combination with the current view mode.'),
    '#default_value' => $layout->label,
  );
  
  $form['additional_settings']['components_layout_settings']['component_layout_image'] = array(
    '#type' => 'managed_file',
    '#title' => t('Layout image'),
    '#description' => t('A sample image of how the component type may look like in the current view mode.'),
    '#default_value' => $layout->image,
    '#upload_location' => 'public://component-type-layouts/',
    '#upload_validators' => array(
      'file_validate_extensions' => array('png', 'gif', 'jpg', 'jpeg'),
    ),
  );
  
  $form['#validate'][] = 'components_layout_settings_validate';
  $form['#submit'][] = 'components_layout_settings_save';
}

function components_layout_settings_validate($form, &$form_state) {
  // @TODO: check if view mode alias is unique for the component type
}

function components_layout_settings_save($form, &$form_state) {
  ctools_include('export');

  $old_layout = $form_state['complete form']['#components_layout'];
  $new_layout = $old_layout;
  
  // Delete the existing layout (if it exists)
  ctools_export_crud_delete('components_layout', $old_layout);
  
  $add_settings = &$form_state['values']['additional_settings'];
  $new_layout->label = $add_settings['component_layout_label'];
  $new_layout->image = $add_settings['component_layout_image'];
  
  $image_changed = ($old_layout->image == $new_layout->image) ? FALSE : TRUE;
  if ($image_changed) {
    // Delete old file
    $old_file = file_load($old_layout->image);
    file_delete($old_file);
    // Save new file (permanently)
    $new_file = file_load($new_layout->image);
    $new_file->status = FILE_STATUS_PERMANENT;
    file_save($new_file);
  }
  
  // Save the new/updated layout
  ctools_export_crud_save('components_layout', $new_layout);
}
