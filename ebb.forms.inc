<?php

/**
 * @file
 * Entity Building Block forms
 */

function ebb_form($form, &$form_state, $ebb) {
  $form['type'] = array(
    '#type' => 'hidden',
    '#value' => $ebb->type,
  );
  
  $form['status'] = array(
    '#type' => 'checkbox',
    '#title' => t('Published'),
    '#default_value' => $ebb->status,
    '#weight' => 9999,
  );
  
  $form['actions'] = array();
  $form['actions']['#weight'] = 1000;
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#submit' => array('ebb_form_submit'),
  );
  
  if (!empty($ebb->ebbid)) {
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#submit' => array('ebb_form_delete_submit'),
    );
  }
  
  field_attach_form('ebb', $ebb, $form, $form_state);
  
  return $form;
}

function ebb_form_validate($form, &$form_state) {
  $ebb = ebb_create($form_state['values']);
  $form_state['entity'] = $ebb;
  
  // Field validation
  field_attach_form_validate('ebb', $ebb, $form, $form_state);
}

function ebb_form_submit($form, &$form_state) {
  //$ebb = ebb_create($form_state['values']);
  $ebb = $form_state['entity'];
  
  // Notify field widgets
  field_attach_submit('ebb', $ebb, $form, $form_state);
  
  // Save the Entity Building Block
  ebb_save($ebb);
  
  $form_state['redirect'] = $ebb->defaultUri();
}

function ebb_form_delete_submit($form, &$form_state) {
  $destination = array();
  if (isset($_GET['destination'])) {
    $destination = drupal_get_destination();
    unset($_GET['destination']);
  }
  $ebb = $form_state['entity'];
  $form_state['redirect'] = array($ebb->defaultUri() . '/delete', array('query' => $destination));
}

function ebb_form_delete_confirm($form, &$form_state, $ebb) {
  $form['ebbid'] = array(
    '#type' => 'hidden',
    '#value' => $ebb->ebbid,
  );
  
  return confirm_form(
    $form,
    t('Are you sure you want to delete this %type Entity Building Block?', array('%type' => ebb_type_get_label_by_type($ebb->type))),
    $ebb->defaultUri(),
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

function ebb_form_delete_confirm_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    $ebb = ebb_load($form_state['values']['ebbid']);
    ebb_delete($ebb->ebbid);
    watchdog('ebb', 'EBB: deleted ebbid=%ebbid', array('%ebbid' => $ebb->ebbid));
    drupal_set_message(t('The %type Entity Building Block has been deleted', array('%type' => ebb_type_get_label_by_type($ebb->type))));
  }
  
  $form_state['redirect'] = '<front>';
}
