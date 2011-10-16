<?php

/**
 * @file
 * Component forms
 */

/**
 * Implements hook_forms().
 * All component forms share the same form handler.
 */
function components_forms() {
  $forms = array();
  if ($types = components_get_types_names()) {
    foreach (array_keys($types) as $type) {
      $forms[$type . '_component_form']['callback'] = 'component_form';
    }
  }
  return $forms;
}

function component_form($form, &$form_state) {
  $component = $form_state['component'];
  
  if (!empty($component->cid)) {
    $form_state['title'] = t('Edit @type component', array('@type' => component_type_get_label_by_type($component->type)));
    
    $form['cid'] = array( // component_save() needs this to determine it's an existing component
      '#type' => 'hidden',
      '#value' => $component->cid,
    );
  }
  else {
    $form_state['title'] = t('Create new @type component', array('@type' => component_type_get_label_by_type($component->type)));
   
    $form['host_entity_type'] = array(
      '#type' => 'hidden',
      '#value' => $form_state['host_entity_type'],
    );
    
    $form['host_entity_id'] = array(
      '#type' => 'hidden',
      '#value' => $form_state['host_entity_id'],
    );
    
    $form['host_entity_field_name'] = array(
      '#type' => 'hidden',
      '#value' => $form_state['host_entity_field_name'],
    );
  }
    
  $form['type'] = array(
    '#type' => 'hidden',
    '#value' => $component->type,
  );
  
  $form['view_mode'] = array(
    '#type' => 'hidden',
    '#value' => $component->view_mode,
  );
  
  field_attach_form('component', $component, $form, $form_state);
  
  return $form;
}

function component_form_validate($form, &$form_state) {
  $component = component_create($form_state['values']);
  $form_state['component'] = $component;
  
  // Field validation
  field_attach_form_validate('component', $component, $form, $form_state);
}

function component_form_submit($form, &$form_state) {
  $component = $form_state['component'];
  
  // Notify field widgets
  if (empty($component->cid)) {
    field_attach_submit('component', $component, $form, $form_state);
  }
  else {
    field_attach_update('component', $component);
  }
  
  // Save the component
  component_save($component);
  
  drupal_set_message(t('The changes have been saved.'));
}

// @TODO: check if destination works
function component_form_delete_submit($form, &$form_state) {
  $destination = array();
  if (isset($_GET['destination'])) {
    $destination = drupal_get_destination();
    unset($_GET['destination']);
  }
  $component = $form_state['entity'];
  $redirect = $component->defaultUri();
  $redirect['path'] = $redirect['path'] . '/delete';
  $redirect['query'] = $destination;
  $form_state['redirect'] = $redirect;
}

function component_form_delete_confirm($form, &$form_state, $component) {
  $form['cid'] = array(
    '#type' => 'hidden',
    '#value' => $component->cid,
  );
  
  return confirm_form(
    $form,
    t('Are you sure you want to delete this %type component?', array('%type' => component_type_get_label_by_type($component->type))),
    $component->defaultUri(),
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

function component_form_delete_confirm_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    $component = component_load($form_state['values']['cid']);
    component_delete($component->cid);
    watchdog('component', 'Component: deleted cid=@cid', array('@cid' => $component->cid));
    drupal_set_message(t('The %type component has been deleted.', array('%type' => component_type_get_label_by_type($component->type))));
  }
  
  $destination = array();
  if (isset($_GET['destination'])) {
    $destination = drupal_get_destination();
    unset($_GET['destination']);
  }
  
  $form_state['redirect'] = ($destination) ? $destination : '<front>';
}
