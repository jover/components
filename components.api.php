<?php

/**
 * @file
 * This file contains no working PHP code; it exists to provide additional
 * documentation for doxygen as well as to document hooks in the standard
 * Drupal manner.
 */

/**
 * @addtogroup hooks
 * @{
 */

/***********************/
/* COMPONENT functions */
/***********************/

/**
 * Acts o component being loaded from the database.
 *
 * This hook is invoked during component loading, which is handled by
 * entity_load(), via the EntityCRUDController.
 *
 * @param $entities
 *   An array of component entities being loaded, keyed by id.
 *
 * @see hook_entity_load()
 */
function hook_component_load($entities) {
  $result = db_query('SELECT pid, foo FROM {mytable} WHERE pid IN(:ids)', array(':ids' => array_keys($entities)));
  foreach ($result as $record) {
    $entities[$record->pid]->foo = $record->foo;
  }
}

/**
 * Responds when a component is inserted.
 *
 * This hook is invoked after the component is inserted into the database.
 *
 * @param $component
 *   The component that is being inserted.
 *
 * @see hook_entity_insert()
 */
function hook_component_insert($component) {
  db_insert('mytable')
    ->fields(array(
      'pid' => $component->pid,
      'extra' => $component->extra,
    ))
    ->execute();
}

/**
 * Acts on a component being inserted or updated.
 *
 * This hook is invoked before the component is saved to the database.
 *
 * @param $component
 *   The component that is being inserted or updated.
 *
 * @see hook_entity_presave()
 */
function hook_component_presave($component) {
  $component->extra = 'foo';
}

/**
 * Responds to a component being componented.
 *
 * This hook is invoked after the component has been updated in the database.
 *
 * @param $component
 *   The component that is being updated.
 *
 * @see hook_entity_update()
 */
function hook_component_update($component) {
  db_update('mytable')
    ->fields(array('extra' => $component->extra))
    ->condition('pid', $component->pid)
    ->execute();
}

/**
 * Responds to component deletion.
 *
 * This hook is invoked after the component has been removed from the database.
 *
 * @param $component
 *   The component that is being deleted.
 *
 * @see hook_entity_delete()
 */
function hook_component_delete($component) {
  db_delete('mytable')
    ->condition('pid', $component->pid)
    ->execute();
}

/**
 * Act on a component that is being assembled before rendering.
 *
 * @param $component
 *   The component entity.
 * @param $view_mode
 *   The view mode the component is rendered in.
 * @param $langcode
 *   The language code used for rendering.
 *
 * The module may add elements to $component->content prior to rendering. The
 * structure of $component->content is a renderable array as expected by
 * drupal_render().
 *
 * @see hook_entity_prepare_view()
 * @see hook_entity_view()
 */
function hook_component_view($component, $view_mode, $langcode) {
  $component->content['my_additional_field'] = array(
    '#markup' => $additional_field,
    '#weight' => 10,
    '#theme' => 'mymodule_my_additional_field',
  );
}

/**
 * Alter the results of entity_view() for components.
 *
 * @param $build
 *   A renderable array representing the component content.
 *
 * This hook is called after the content has been assembled in a structured
 * array and may be used for doing processing which requires that the complete
 * component content structure has been built.
 *
 * If the module wishes to act on the rendered HTML of the component rather than
 * the structured content array, it may use this hook to add a #post_render
 * callback. Alternatively, it could also implement hook_preprocess_component().
 * See drupal_render() and theme() documentation respectively for details.
 *
 * @see hook_entity_view_alter()
 */
function hook_component_view_alter($build) {
  if ($build['#view_mode'] == 'full' && isset($build['an_additional_field'])) {
    // Change its weight.
    $build['an_additional_field']['#weight'] = -10;

    // Add a #post_render callback to act on the rendered HTML of the entity.
    $build['#post_render'][] = 'my_module_post_render';
  }
}

/**
 * Define default component configurations.
 *
 * @return
 *   An array of default components, keyed by component names.
 *
 * @see hook_default_component_alter()
 */
/*function hook_default_component() {
  $defaults['main'] = entity_create('component', array(
    'label' => t('Entity example'),
    'weight' => 0,
  ));
  return $defaults;
}*/

/**
 * Alter default component configurations.
 *
 * @param $defaults
 *   An array of default components, keyed by component names.
 *
 * @see hook_default_component()
 */
/*function hook_default_component_alter(&$defaults) {
  $defaults['main']->label = 'custom label';
}*/

/**
 * Alter component forms.
 *
 * Modules may alter the component entity form by making use of this hook or
 * the entity bundle specific hook_form_component_edit_BUNDLE_form_alter().
 * #entity_builders may be used in order to copy the values of added form
 * elements to the entity, just as documented for
 * entity_form_submit_build_entity().
 *
 * @param $form
 *   Nested array of form elements that comprise the form.
 * @param $form_state
 *   A keyed array containing the current state of the form.
 */
/*function hook_form_component_form_alter(&$form, &$form_state) {
  // Your alterations.
}*/

/****************************/
/* COMPONENT_TYPE functions */
/****************************/

/**
 * Acts on component type being loaded from the database.
 *
 * This hook is invoked during component type loading, which is handled by
 * entity_load(), via the EntityCRUDController.
 *
 * @param $entities
 *   An array of component type entities being loaded, keyed by id.
 *
 * @see hook_entity_load()
 */
function hook_component_type_load($entities) {
  /*$result = db_query('SELECT pid, foo FROM {mytable} WHERE pid IN(:ids)', array(':ids' => array_keys($entities)));
  foreach ($result as $record) {
    $entities[$record->pid]->foo = $record->foo;
  }*/
  if (isset($types['main'])) {
    
  }
}

/**
 * Responds when a component type is inserted.
 *
 * This hook is invoked after the component type is inserted into the database.
 *
 * @param $component_type
 *   The component type that is being inserted.
 *
 * @see hook_entity_insert()
 */
function hook_component_type_insert($component_type) {
  db_insert('mytable')
    ->fields(array(
      'pid' => $component_type->pid,
      'extra' => $component_type->extra,
    ))
    ->execute();
}

/**
 * Acts on a component type being inserted or updated.
 *
 * This hook is invoked before the component type is saved to the database.
 *
 * @param $component_type
 *   The component type that is being inserted or updated.
 *
 * @see hook_entity_presave()
 */
function hook_component_type_presave($component_type) {
  $component_type->extra = 'foo';
}

/**
 * Responds to a component type being updated.
 *
 * This hook is invoked after the component type has been updated in the database.
 *
 * @param $component_type
 *   The component type that is being updated.
 *
 * @see hook_entity_update()
 */
function hook_component_type_update($component_type) {
  db_update('mytable')
    ->fields(array('extra' => $component_type->extra))
    ->condition('pid', $component_type->pid)
    ->execute();
}

/**
 * Responds to component type deletion.
 *
 * This hook is invoked after the component type has been removed from the database.
 *
 * @param $component_type
 *   The component type that is being deleted.
 *
 * @see hook_entity_delete()
 */
function hook_component_type_delete($component_type) {
  db_delete('mytable')
    ->condition('pid', $component_type->pid)
    ->execute();
}

/**
 * Act on a component type that is being assembled before rendering.
 *
 * @param $component_type
 *   The component type entity.
 * @param $view_mode
 *   The view mode the component type is rendered in.
 * @param $langcode
 *   The language code used for rendering.
 *
 * The module may add elements to $component_type->content prior to rendering. The
 * structure of $component_type->content is a renderable array as expected by
 * drupal_render().
 *
 * @see hook_entity_prepare_view()
 * @see hook_entity_view()
 */
/*function hook_component_type_view($component_type, $view_mode, $langcode) {
  $component_type->content['my_additional_field'] = array(
    '#markup' => $additional_field,
    '#weight' => 10,
    '#theme' => 'mymodule_my_additional_field',
  );
}*/

/**
 * Alter the results of entity_view() for component types.
 *
 * @param $build
 *   A renderable array representing the component type content.
 *
 * This hook is called after the content has been assembled in a structured
 * array and may be used for doing processing which requires that the complete
 * component type content structure has been built.
 *
 * If the module wishes to act on the rendered HTML of the component type rather than
 * the structured content array, it may use this hook to add a #post_render
 * callback. Alternatively, it could also implement hook_preprocess_component_type().
 * See drupal_render() and theme() documentation respectively for details.
 *
 * @see hook_entity_view_alter()
 */
/*function hook_component_type_view_alter($build) {
  if ($build['#view_mode'] == 'full' && isset($build['an_additional_field'])) {
    // Change its weight.
    $build['an_additional_field']['#weight'] = -10;

    // Add a #post_render callback to act on the rendered HTML of the entity.
    $build['#post_render'][] = 'my_module_post_render';
  }
}*/

/**
 * Define default component type configurations.
 *
 * @return
 *   An array of default component types, keyed by component type names.
 *
 * @see hook_default_component_type_alter()
 */
function hook_default_component_type() {
  $types['main'] = new ComponentType(array(
    'type' => 'main',
    'label' => t('Component'),
    'weight' => 0,
    'locked' => TRUE,
  ));
  return $types;
}

/**
 * Alter default component type configurations.
 *
 * @param $defaults
 *   An array of default component types, keyed by component type names.
 *
 * @see hook_default_component_type()
 */
function hook_default_component_type_alter(&$defaults) {
  $defaults['main']->label = 'custom label';
}

/**
 * Alter component type forms.
 *
 * Modules may alter the component type entity form by making use of this hook or
 * the entity bundle specific hook_form_component_type_edit_BUNDLE_form_alter().
 * #entity_builders may be used in order to copy the values of added form
 * elements to the entity, just as documented for
 * entity_form_submit_build_entity().
 *
 * @param $form
 *   Nested array of form elements that comprise the form.
 * @param $form_state
 *   A keyed array containing the current state of the form.
 */
function hook_form_component_type_form_alter(&$form, &$form_state) {
  // Your alterations.
}
 
/**
 * @}
 */
