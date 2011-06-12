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

/*****************/
/* EBB functions */
/*****************/

/**
 * Acts on Entity Building Block being loaded from the database.
 *
 * This hook is invoked during Entity Building Block loading, which is handled by
 * entity_load(), via the EntityCRUDController.
 *
 * @param $entities
 *   An array of Entity Building Block entities being loaded, keyed by id.
 *
 * @see hook_entity_load()
 */
function hook_ebb_load($entities) {
  $result = db_query('SELECT pid, foo FROM {mytable} WHERE pid IN(:ids)', array(':ids' => array_keys($entities)));
  foreach ($result as $record) {
    $entities[$record->pid]->foo = $record->foo;
  }
}

/**
 * Responds when a Entity Building Block is inserted.
 *
 * This hook is invoked after the Entity Building Block is inserted into the database.
 *
 * @param $ebb
 *   The Entity Building Block that is being inserted.
 *
 * @see hook_entity_insert()
 */
function hook_ebb_insert($ebb) {
  db_insert('mytable')
    ->fields(array(
      'pid' => $ebb->pid,
      'extra' => $ebb->extra,
    ))
    ->execute();
}

/**
 * Acts on a Entity Building Block being inserted or updated.
 *
 * This hook is invoked before the Entity Building Block is saved to the database.
 *
 * @param $ebb
 *   The Entity Building Block that is being inserted or updated.
 *
 * @see hook_entity_presave()
 */
function hook_ebb_presave($ebb) {
  $ebb->extra = 'foo';
}

/**
 * Responds to a Entity Building Block being updated.
 *
 * This hook is invoked after the Entity Building Block has been updated in the database.
 *
 * @param $ebb
 *   The Entity Building Block that is being updated.
 *
 * @see hook_entity_update()
 */
function hook_ebb_update($ebb) {
  db_update('mytable')
    ->fields(array('extra' => $ebb->extra))
    ->condition('pid', $ebb->pid)
    ->execute();
}

/**
 * Responds to Entity Building Block deletion.
 *
 * This hook is invoked after the Entity Building Block has been removed from the database.
 *
 * @param $ebb
 *   The Entity Building Block that is being deleted.
 *
 * @see hook_entity_delete()
 */
function hook_ebb_delete($ebb) {
  db_delete('mytable')
    ->condition('pid', $ebb->pid)
    ->execute();
}

/**
 * Act on a Entity Building Block that is being assembled before rendering.
 *
 * @param $ebb
 *   The Entity Building Block entity.
 * @param $view_mode
 *   The view mode the Entity Building Block is rendered in.
 * @param $langcode
 *   The language code used for rendering.
 *
 * The module may add elements to $ebb->content prior to rendering. The
 * structure of $ebb->content is a renderable array as expected by
 * drupal_render().
 *
 * @see hook_entity_prepare_view()
 * @see hook_entity_view()
 */
function hook_ebb_view($ebb, $view_mode, $langcode) {
  $ebb->content['my_additional_field'] = array(
    '#markup' => $additional_field,
    '#weight' => 10,
    '#theme' => 'mymodule_my_additional_field',
  );
}

/**
 * Alter the results of entity_view() for Entity Building Blocks.
 *
 * @param $build
 *   A renderable array representing the Entity Building Block content.
 *
 * This hook is called after the content has been assembled in a structured
 * array and may be used for doing processing which requires that the complete
 * Entity Building Block content structure has been built.
 *
 * If the module wishes to act on the rendered HTML of the Entity Building Block rather than
 * the structured content array, it may use this hook to add a #post_render
 * callback. Alternatively, it could also implement hook_preprocess_ebb().
 * See drupal_render() and theme() documentation respectively for details.
 *
 * @see hook_entity_view_alter()
 */
function hook_ebb_view_alter($build) {
  if ($build['#view_mode'] == 'full' && isset($build['an_additional_field'])) {
    // Change its weight.
    $build['an_additional_field']['#weight'] = -10;

    // Add a #post_render callback to act on the rendered HTML of the entity.
    $build['#post_render'][] = 'my_module_post_render';
  }
}

/**
 * Define default Entity Building Block configurations.
 *
 * @return
 *   An array of default Entity Building Blocks, keyed by Entity Building Block names.
 *
 * @see hook_default_ebb_alter()
 */
/*function hook_default_ebb() {
  $defaults['main'] = entity_create('ebb', array(
    'label' => t('Entity example'),
    'weight' => 0,
  ));
  return $defaults;
}*/

/**
 * Alter default Entity Building Block configurations.
 *
 * @param $defaults
 *   An array of default Entity Building Blocks, keyed by Entity Building Block names.
 *
 * @see hook_default_ebb()
 */
/*function hook_default_ebb_alter(&$defaults) {
  $defaults['main']->label = 'custom label';
}*/

/**
 * Alter Entity Building Block forms.
 *
 * Modules may alter the Entity Building Block entity form by making use of this hook or
 * the entity bundle specific hook_form_ebb_edit_BUNDLE_form_alter().
 * #entity_builders may be used in order to copy the values of added form
 * elements to the entity, just as documented for
 * entity_form_submit_build_entity().
 *
 * @param $form
 *   Nested array of form elements that comprise the form.
 * @param $form_state
 *   A keyed array containing the current state of the form.
 */
/*function hook_form_ebb_form_alter(&$form, &$form_state) {
  // Your alterations.
}*/

/**********************/
/* EBB_TYPE functions */
/**********************/

/**
 * Acts on Entity Building Block Type being loaded from the database.
 *
 * This hook is invoked during Entity Building Block Type loading, which is handled by
 * entity_load(), via the EntityCRUDController.
 *
 * @param $entities
 *   An array of Entity Building Block Type entities being loaded, keyed by id.
 *
 * @see hook_entity_load()
 */
function hook_ebb_type_load($entities) {
  /*$result = db_query('SELECT pid, foo FROM {mytable} WHERE pid IN(:ids)', array(':ids' => array_keys($entities)));
  foreach ($result as $record) {
    $entities[$record->pid]->foo = $record->foo;
  }*/
  if (isset($types['main'])) {
    
  }
}

/**
 * Responds when a Entity Building Block Type is inserted.
 *
 * This hook is invoked after the Entity Building Block Type is inserted into the database.
 *
 * @param $ebb_type
 *   The Entity Building Block Type that is being inserted.
 *
 * @see hook_entity_insert()
 */
function hook_ebb_type_insert($ebb_type) {
  db_insert('mytable')
    ->fields(array(
      'pid' => $ebb_type->pid,
      'extra' => $ebb_type->extra,
    ))
    ->execute();
}

/**
 * Acts on a Entity Building Block Type being inserted or updated.
 *
 * This hook is invoked before the Entity Building Block Type is saved to the database.
 *
 * @param $ebb_type
 *   The Entity Building Block Type that is being inserted or updated.
 *
 * @see hook_entity_presave()
 */
function hook_ebb_type_presave($ebb_type) {
  $ebb_type->extra = 'foo';
}

/**
 * Responds to a Entity Building Block Type being updated.
 *
 * This hook is invoked after the Entity Building Block Type has been updated in the database.
 *
 * @param $ebb_type
 *   The Entity Building Block Type that is being updated.
 *
 * @see hook_entity_update()
 */
function hook_ebb_type_update($ebb_type) {
  db_update('mytable')
    ->fields(array('extra' => $ebb_type->extra))
    ->condition('pid', $ebb_type->pid)
    ->execute();
}

/**
 * Responds to Entity Building Block Type deletion.
 *
 * This hook is invoked after the Entity Building Block Type has been removed from the database.
 *
 * @param $ebb_type
 *   The Entity Building Block Type that is being deleted.
 *
 * @see hook_entity_delete()
 */
function hook_ebb_type_delete($ebb_type) {
  db_delete('mytable')
    ->condition('pid', $ebb_type->pid)
    ->execute();
}

/**
 * Act on a Entity Building Block Type that is being assembled before rendering.
 *
 * @param $ebb_type
 *   The Entity Building Block Type entity.
 * @param $view_mode
 *   The view mode the Entity Building Block Type is rendered in.
 * @param $langcode
 *   The language code used for rendering.
 *
 * The module may add elements to $ebb_type->content prior to rendering. The
 * structure of $ebb_type->content is a renderable array as expected by
 * drupal_render().
 *
 * @see hook_entity_prepare_view()
 * @see hook_entity_view()
 */
/*function hook_ebb_type_view($ebb_type, $view_mode, $langcode) {
  $ebb_type->content['my_additional_field'] = array(
    '#markup' => $additional_field,
    '#weight' => 10,
    '#theme' => 'mymodule_my_additional_field',
  );
}*/

/**
 * Alter the results of entity_view() for Entity Building Block Types.
 *
 * @param $build
 *   A renderable array representing the Entity Building Block Type content.
 *
 * This hook is called after the content has been assembled in a structured
 * array and may be used for doing processing which requires that the complete
 * Entity Building Block Type content structure has been built.
 *
 * If the module wishes to act on the rendered HTML of the Entity Building Block Type rather than
 * the structured content array, it may use this hook to add a #post_render
 * callback. Alternatively, it could also implement hook_preprocess_ebb_type().
 * See drupal_render() and theme() documentation respectively for details.
 *
 * @see hook_entity_view_alter()
 */
/*function hook_ebb_type_view_alter($build) {
  if ($build['#view_mode'] == 'full' && isset($build['an_additional_field'])) {
    // Change its weight.
    $build['an_additional_field']['#weight'] = -10;

    // Add a #post_render callback to act on the rendered HTML of the entity.
    $build['#post_render'][] = 'my_module_post_render';
  }
}*/

/**
 * Define default Entity Building Block Type configurations.
 *
 * @return
 *   An array of default Entity Building Block Types, keyed by Entity Building Block Type names.
 *
 * @see hook_default_ebb_type_alter()
 */
function hook_default_ebb_type() {
  $types['main'] = new EntityBuildingBlockType(array(
    'type' => 'main',
    'label' => t('Entity Building Block'),
    'weight' => 0,
    'locked' => TRUE,
  ));
  return $types;
}

/**
 * Alter default Entity Building Block Type configurations.
 *
 * @param $defaults
 *   An array of default Entity Building Block Types, keyed by Entity Building Block Type names.
 *
 * @see hook_default_ebb_type()
 */
function hook_default_ebb_type_alter(&$defaults) {
  $defaults['main']->label = 'custom label';
}

/**
 * Alter Entity Building Block Type forms.
 *
 * Modules may alter the Entity Building Block Type entity form by making use of this hook or
 * the entity bundle specific hook_form_ebb_type_edit_BUNDLE_form_alter().
 * #entity_builders may be used in order to copy the values of added form
 * elements to the entity, just as documented for
 * entity_form_submit_build_entity().
 *
 * @param $form
 *   Nested array of form elements that comprise the form.
 * @param $form_state
 *   A keyed array containing the current state of the form.
 */
function hook_form_ebb_type_form_alter(&$form, &$form_state) {
  // Your alterations.
}
 
/**
 * @}
 */
