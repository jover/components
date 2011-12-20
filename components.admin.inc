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
    
    return parent::overviewTableRow($conditions, $id, $entity, $additional_cols);
  }
  
}

 /**
  * Generates the component editing form.
  */
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
  }
  
  $form['type'] = array(
    '#type' => 'hidden',
    '#value' => $component->type,
  );
  
  $form['view_mode'] = array(
    '#type' => 'hidden',
    '#value' => $component->view_mode,
  );

  $form['host_entity_type'] = array(
    '#type' => 'hidden',
    '#value' => $component->host_entity_type,
  );
  
  $form['host_entity_id'] = array(
    '#type' => 'hidden',
    '#value' => $component->host_entity_id,
  );
  
  $form['host_entity_group'] = array(
    '#type' => 'hidden',
    '#value' => $component->host_entity_group,
  );

  field_attach_form('component', $component, $form, $form_state);
  
  return $form;
}

/**
 * Form API validate callback for the component form.
 */
function component_form_validate($form, &$form_state) {
  $component = component_create($form_state['values']);
  
  // Field validation
  field_attach_form_validate('component', $component, $form, $form_state);
  
  $form_state['component'] = $component;
}

/**
 * Form API submit callback for the component form.
 */
function component_form_submit($form, &$form_state) {
  $component = $form_state['component'];
  
  entity_form_submit_build_entity('component', $component, $form, $form_state);
  
  if (empty($component->cid)) {
    $component->is_new = TRUE;
  }
  
  // Save the component
  component_save($component);
  
  drupal_set_message(t('The changes have been saved.'));
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
 * Form API submit callback for the component type form.
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
 * Store our little cache so that we can retain data from form to form.
 */
function component_wizard_cache_set($id, $component) {
  ctools_include('object-cache');
  ctools_object_cache_set('component_wizard', $id, $component);
}

/**
 * Get the current object from the cache, or default.
 */
function component_wizard_cache_get($id) {
  ctools_include('object-cache');
  $component = ctools_object_cache_get('component_wizard', $id);
  if (!$component) {
    // Create a default component.
    $component = new stdClass();
    $component->type = 'unknown';
    $component->view_mode = NULL;
    $component->layout = NULL;
  }

  return $component;
}

/**
 * Clear the wizard cache.
 */
function component_wizard_cache_clear($id) {
  ctools_include('object-cache');
  ctools_object_cache_clear('component_wizard', $id);
}

/**
 * Handle the 'finish' click on teh add/edit pane form wizard.
 *
 * All we need to do is set a flag so the return can handle adding
 * the pane.
 */
function component_wizard_finish(&$form_state) {
  $form_state['complete'] = TRUE;
}

/**
 * Handle the 'cancel' click on the add/edit pane form wizard.
 */
function component_wizard_cancel(&$form_state) {
  $form_state['cancel'] = TRUE;
}

function component_wizard_add($entity_type, $entity_id, $group, $js = NULL, $step = NULL) {
  if ($js) {
    ctools_include('ajax');
    ctools_include('modal');
  }
  
  $form_info = array(
    'id' => 'component-add',
    'path' => 'component/add/' . $entity_type . '/' . $entity_id . '/' . $group . '/' . ($js ? 'ajax' : 'nojs') . '/%step',
    'show trail' => TRUE,
    'show back' => TRUE,
    'show cancel' => TRUE,
    'show return' => FALSE,
    'next callback' =>  'component_wizard_add_next',
    'finish callback' => 'component_wizard_finish',
    'cancel callback' => 'component_wizard_cancel',
    // this controls order, as well as form labels
    'order' => array(
      /*'type' => t('Choose component type'),
      'view_mode' => t('Choose view mode'),*/
      'component_type_layout' => t('Choose component type and layout'),
    ),
    // here we map a step to a form id.
    'forms' => array(
      // e.g. this for the step at wombat/create
      /*'type' => array(
        'form id' => 'component_wizard_add_component_type',
      ),
      'view_mode' => array(
        'form id' => 'component_wizard_add_component_view_mode',
      ),*/
      'component_type_layout' => array(
        'form id' => 'component_wizard_add_component_type_layout',
      ),
    ),
  );
  
  $component_id = 1;
  
  if (empty($step)) {
    // We reset the form when $step is NULL because that means they have
    // for whatever reason started over.
    component_wizard_cache_clear($component_id);
    //$step = 'type';
    $step = 'component_type_layout';
  }
  
  // This automatically gets defaults if there wasn't anything saved.
  $component = component_wizard_cache_get($component_id);
  $component->host_entity_type = $entity_type;
  $component->host_entity_id = $entity_id;
  $component->host_entity_group = $group;
  
  $component_types = components_get_types_names();
  
  // Make sure we can't somehow accidentally go to an invalid layout.
  if (empty($component_types[$component->type])) {
    $component->type = 'unknown';
  }
  
  // Now that we have our object, dynamically add the animal's form.
  if ($component->type == 'unknown') {
    // If they haven't selected a type, add a form that doesn't exist yet.
    $form_info['order']['unknown'] = t('Configure component');
    $form_info['forms']['unknown'] = array('form id' => 'nothing');
  }
  else {
    // Add the selected component type to the order so that it shows up properly in the trail.
    $form_info['order'][$component->type] = $component_types[$component->type];
  }
  
  foreach ($component_types as $type => $label) {
    $form_info['forms'][$type] = array(
      // $type . '_component_form' doesn't work because the corresponding
      // submit function cannot be found by the CTools Wizard functions
      'form id' => 'component_form',
      //'include' => $module_path . '/components.forms.inc',
    );
  }
  
  $form_state = array(
    'ajax' => $js,
    // Put our component and ID into the form state cache so we can easily find it.
    'component_id' => $component_id,
    'component' => &$component,
  );
  
  // Send this all off to our form. This is like drupal_get_form only wizardy.
  ctools_include('wizard');
  $form = ctools_wizard_multistep_form($form_info, $step, $form_state);
  $output = drupal_render($form);
  
  // If $output is FALSE, there was no actual form.
  if ($js) {
    // If javascript is active, we have to use a render array.
    $commands = array();
    if ($output === FALSE || !empty($form_state['complete'])) {
      // Dismiss the modal.
      $commands[] = ctools_modal_command_dismiss();
    }
    elseif (!empty($form_state['cancel'])) {
      // If cancelling, return to the activity.
      $commands[] = ctools_modal_command_dismiss();
    }
    else {
      $commands = ctools_modal_form_render($form_state, $output);
    }
    print ajax_render($commands);
    exit;
  }
  else {
    if ($output === FALSE || !empty($form_state['complete'])) {
      return $component;
    }
    elseif (!empty($form_state['cancel'])) {
      // @TODO: change node to appropriate entity type
      drupal_goto('node/' . $entity_id);
    }
    else {
      return $output;
    }
  }
}

/**
 * Handle the 'next' click on the add/edit pane form wizard.
 *
 * All we need to do is store the updated pane in the cache.
 */
function component_wizard_add_next(&$form_state) {
  component_wizard_cache_set($form_state['component_id'], $form_state['component']);
}

function component_wizard_add_component_type($form, &$form_state) {
  $form_state['title'] = t('Choose component type');
  $options = array();
  
  foreach (components_get_types_names() as $type => $label) {
    $options[$type] = $label;
  }
  
  $form['type'] = array(
    '#title' => t('Choose component type'),
    '#type' => 'radios',
    '#options' => $options,
    '#default_value' => $form_state['component']->type,
    '#required' => TRUE,
  );
  
  return $form;
}

function component_wizard_add_component_type_submit(&$form, &$form_state) {
  $form_state['component']->type = $form_state['values']['type'];
  // Override where to go next based on the component type selected.
  $form_state['clicked_button']['#next'] = 'view_mode';
}

function component_wizard_add_component_view_mode($form, &$form_state) {
  $form_state['title'] = t('Choose view mode');
  $component_type = $form_state['component']->type;
  $options = array();
  
  $entity_info = entity_get_info('component');
  foreach ($entity_info['view modes'] as $view_mode => $properties) {
    $options[$view_mode] = $properties['label'];
  }
  
  $form['view_mode'] = array(
    '#title' => t('Choose view mode'),
    '#type' => 'radios',
    '#options' => $options,
    '#default_value' => $form_state['component']->view_mode,
    '#required' => TRUE,
  );
  
  return $form;
}

function component_wizard_add_component_view_mode_submit(&$form, &$form_state) {
  $form_state['component']->view_mode = $form_state['values']['view_mode'];
  // Override where to go next based on the component view mode selected.
  $form_state['clicked_button']['#next'] = $form_state['component']->type;
}

function component_wizard_add_component_type_layout($form, &$form_state) {
  module_load_include('inc', 'components', 'components.layouts');
  
  $form_state['title'] = t('Choose component type and layout');
  
  $layouts = components_get_layouts();
  
  $bundles = array();
  foreach ($layouts as $id => $layout) {
    if (isset($layout->bundle)) {
      $bundle = $layout->bundle;
      $bundles[$bundle][$id] = $layout;
    }
  }
  
  $form['component_types'] = array(
    '#type' => 'vertical_tabs',
  );
  
  foreach ($bundles as $bundle => $bundle_layouts) {
    $bundle_label = component_type_get_label_by_type($bundle);
    
    $form[$bundle] = array(
      '#type' => 'fieldset',
      '#title' => ($bundle_label) ? $bundle_label : $bundle,
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#group' => 'component_types',
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
      
      $form[$bundle][$id] = array(
        '#type' => 'radio',
        '#title' => $image . '<span>' . $layout->label . '</span>',
        '#return_value' => $id,
        '#default_value' => FALSE, // @TODO: check default value
        '#parents' => array('component_type_layout'),
      );
    }
  }

  return $form;
}

function component_wizard_add_component_type_layout_submit(&$form, &$form_state) {
  ctools_include('export');
  $layout_id = $form_state['values']['component_type_layout'];
  $layout = ctools_export_crud_load('components_layout', $layout_id);
  $form_state['component']->layout = $layout->id;
  $form_state['component']->type = $layout->bundle;
  $form_state['component']->view_mode = $layout->view_mode;
  // Override where to go next based on the component layout selected.
  $form_state['clicked_button']['#next'] = $layout->bundle;
}

function component_wizard_edit($component, $js = NULL) {
  if ($js) {
    ctools_include('ajax');
    ctools_include('modal');
  }
  
  $form_info = array(
    'id' => 'component-edit',
    'path' => 'component/' . $component->cid . '/edit/' . ($js ? 'ajax' : 'nojs'),
    'show cancel' => TRUE,
    'finish callback' => 'component_wizard_finish',
    'cancel callback' => 'component_wizard_cancel',
    // this controls order, as well as form labels
    'order' => array(
      'edit' => t('Edit %type component', array('%type' => component_type_get_label_by_type($component->type))),
    ),
    // here we map a step to a form id.
    'forms' => array(
      // e.g. this for the step at wombat/create
      'edit' => array(
        'form id' => 'component_form',
      ),
    ),
  );
  
  $form_state = array(
    'ajax' => $js,
    // Put our component into the form state cache so we can easily find it.
    'component' => &$component,
  );
  
  $step = 'edit';
  
  // Send this all off to our form. This is like drupal_get_form only wizardy.
  ctools_include('wizard');
  $form = ctools_wizard_multistep_form($form_info, $step, $form_state);
  $output = drupal_render($form);
  
  // If $output is FALSE, there was no actual form.
  if ($js) {
    // If javascript is active, we have to use a render array.
    $commands = array();
    if ($output === FALSE || !empty($form_state['complete'])) {
      // Dismiss the modal.
      $commands[] = ctools_modal_command_dismiss();
    }
    elseif (!empty($form_state['cancel'])) {
      // If cancelling, return to the activity.
      $commands[] = ctools_modal_command_dismiss();
    }
    else {
      $commands = ctools_modal_form_render($form_state, $output);
    }
    print ajax_render($commands);
    exit;
  }
  else {
    if ($output === FALSE || !empty($form_state['complete'])) {
      return $component;
    }
    elseif (!empty($form_state['cancel'])) {
      // @TODO: change node to appropriate entity type
      // drupal_goto('node/' . $entity_id);
    }
    else {
      return $output;
    }
  }
}

function component_wizard_delete($component, $js = NULL) {
  if ($js) {
    ctools_include('ajax');
    ctools_include('modal');
  }
  
  $form_info = array(
    'id' => 'component-delete',
    'path' => 'component/' . $component->cid . '/delete/' . ($js ? 'ajax' : 'nojs'),
    'show cancel' => TRUE,
    'finish callback' => 'component_wizard_finish',
    'cancel callback' => 'component_wizard_cancel',
    // this controls order, as well as form labels
    'order' => array(
      'type' => t('Delete component'),
    ),
    // here we map a step to a form id.
    'forms' => array(
      // e.g. this for the step at wombat/create
      'delete' => array(
        'form id' => 'component_wizard_delete_component',
      ),
    ),
  );
  
  $form_state = array(
    'ajax' => $js,
    // Put our component into the form state cache so we can easily find it.
    'component' => &$component,
  );
  
  $step = 'delete';
  
  // Send this all off to our form. This is like drupal_get_form only wizardy.
  ctools_include('wizard');
  $form = ctools_wizard_multistep_form($form_info, $step, $form_state);
  $output = drupal_render($form);
  
  // If $output is FALSE, there was no actual form.
  if ($js) {
    // If javascript is active, we have to use a render array.
    $commands = array();
    if ($output === FALSE || !empty($form_state['complete'])) {
      // Dismiss the modal.
      $commands[] = ctools_modal_command_dismiss();
    }
    elseif (!empty($form_state['cancel'])) {
      // If cancelling, return to the activity.
      $commands[] = ctools_modal_command_dismiss();
    }
    else {
      $commands = ctools_modal_form_render($form_state, $output);
    }
    print ajax_render($commands);
    exit;
  }
  else {
    if ($output === FALSE || !empty($form_state['complete'])) {
      return $component;
    }
    elseif (!empty($form_state['cancel'])) {
      // @TODO: change node to appropriate entity type
      drupal_goto(drupal_get_destination());
    }
    else {
      return $output;
    }
  }
}

function component_wizard_delete_component($form, &$form_state) {
  $form['confirmation'] = array(
    '#type' => 'markup',
    '#markup' => t('Are you sure you want to delete this %type component?', array('%type' => component_type_get_label_by_type($component->type))),
  );
  
  return $form;
}

function component_wizard_delete_component_submit(&$form, &$form_state) {
  $component = $form_state['component'];
  component_delete($component->cid);
}
