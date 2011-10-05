<?php

/**
 * @file
 * Component forms
 */

function component_form($form, &$form_state, $component, $entity_type = NULL, $entity_id = NULL, $field_name = NULL) {
  $form['type'] = array(
    '#type' => 'hidden',
    '#value' => $component->type,
  );
  
  $form['actions'] = array('#type' => 'actions', '#weight' => 1000);
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#submit' => array('component_form_submit'),
  );
  
  // @TODO: access arguments 'delete any $type component'
  if (!empty($component->cid)) {
    $form['cid'] = array( // component_save() needs this to determine it's an existing component
      '#type' => 'hidden',
      '#value' => $component->cid,
    );
    
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#submit' => array('component_form_delete_submit'),
      //'#access' => user_access('delete any $type component'),
    );
  }
  else {
    $form['host_entity_type'] = array(
      '#type' => 'hidden',
      '#value' => $entity_type,
    );
    
    $form['host_entity_id'] = array(
      '#type' => 'hidden',
      '#value' => $entity_id,
    );
    
    $form['host_entity_field_name'] = array(
      '#type' => 'hidden',
      '#value' => $field_name,
    );
  }
  
  field_attach_form('component', $component, $form, $form_state);
  
  return $form;
}

function component_form_validate($form, &$form_state) {
  $component = component_create($form_state['values']);
  $form_state['entity'] = $component;
  
  // Field validation
  field_attach_form_validate('component', $component, $form, $form_state);
}

function component_form_submit($form, &$form_state) {
  $component = $form_state['entity'];
  
  // Notify field widgets
  field_attach_submit('component', $component, $form, $form_state);
  
  // Save the component
  component_save($component);
  
  drupal_set_message(t('The changes have been saved.'));
  $form_state['redirect'] = $component->defaultUri();
}

function component_ajax_form($form, &$form_state, $component) {
  $form['type'] = array(
    '#type' => 'hidden',
    '#value' => $component->type,
  );
  
  $form['actions'] = array('#type' => 'actions', '#weight' => 1000);
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#submit' => array('component_ajax_form_submit'),
    '#ajax' => array(
      'callback' => 'component_ajax_form_callback',
    ),
  );
  
  // @TODO: access arguments 'delete any $type component'
  if (!empty($component->cid)) {
    $form['cid'] = array( // component_save() needs this to determine it's an existing component
      '#type' => 'hidden',
      '#value' => $component->cid,
    );
    
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#submit' => array('component_form_delete_submit'),
      //'#access' => user_access('delete any $type component'),
    );
  }
  
  field_attach_form('component', $component, $form, $form_state);
  
  return $form;
}

function component_ajax_form_callback($form, $form_state) {
  return $form;
}

function component_ajax_form_validate($form, &$form_state) {
  $component = component_create($form_state['values']);
  $form_state['entity'] = $component;
  
  // Field validation
  field_attach_form_validate('component', $component, $form, $form_state);
}

function component_ajax_form_submit($form, &$form_state) {
  $component = $form_state['entity'];
  
  // Notify field widgets
  field_attach_submit('component', $component, $form, $form_state);
  
  // Save the component
  component_save($component);
  
  drupal_set_message(t('The changes have been saved.'));
  $form_state['rebuild'] = TRUE;
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
