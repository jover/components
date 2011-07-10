<?php

/**
 * @file
 * Defines a field type for referencing a component from another entity.
 * 
 * This module is based on the Node Reference module (part of the References module):
 * @link http://drupal.org/project/references
 */

/************************/
/* Field Type API hooks */
/************************/

/**
 * Implements hook_field_info().
 */
function components_field_info() {
  return array(
    'component_reference' => array(
      'label' => t('Component reference'),
      'description' => t('This field stores the ID of a related component.'),
      'settings' => array( // instance_settings
        'referenceable_types' => array(),
      ),
      'default_widget' => 'component_reference_cid',
      'default_formatter' => 'component_reference_component',
      // Add entity property info
      'property_type' => 'component',
    ),
  );
}

/**
 * Implements hook_field_settings_form().
 */
function components_field_settings_form($field, $instance, $has_data) {
  //$settings = $instance['settings'];
  $settings = $field['settings'];
  
  $form = array();
  $form['referenceable_types'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Component types that can be referenced'),
    '#multiple' => TRUE,
    '#default_value' => is_array($settings['referenceable_types']) ? $settings['referenceable_types'] : array(),
    '#options' => components_get_types_names(),
  );
  
  return $form;
}

/**
 * Implements hook_field_validate().
 */
function components_field_validate($entity_type, $entity, $field, $instance, $langcode, $items, $errors) {
  foreach ($items as $delta => $item) {
    if (!empty($item['cid']) && !is_numeric($item['cid'])) {
      $errors[$field['type']][$langcode][$delta][] = array(
        'error' => 'component_reference_cid_invalid',
        'message' => t('%label: invalid input.', array('%label' => $instance['label'])),
      );
    }
  }
}

/**
 * Implements hook_field_is_empty().
 */
function components_field_is_empty($item, $field) {
  if (!empty($item['cid'])) {
    return FALSE;
  }
  return TRUE;
}

/*****************************/
/* Field Type API: Formatter */
/*****************************/

/**
 * Implements hook_field_formatter_info().
 */
function components_field_formatter_info() {
  return array(
    'component_reference_component' => array(
      'label' => t('Rendered component'),
      'description' => t('Display the referenced component in a specific view mode'),
      'field types' => array('component_reference'),
      'settings' => array(
        'component_reference_view_mode' => 'default',
      ),
    ),
  );
}

/**
 * Implements hook_field_formatter_settings_form().
 */
function components_field_formatter_settings_form($field, $instance, $view_mode, $form, $form_state) {
  $display = $instance['display'][$view_mode];
  $settings = $display['settings'];
  //$settings = $display['instance_settings'];
  //$settings = $instance['settings'];
  
  // Only build this information for component_reference_component displays
  if ($display['type'] !== 'component_reference_component') {
    return NULL;
  }
  
  $entity_info = entity_get_info('component');
  $view_modes = $entity_info['view modes'];
  $options = array();
  
  foreach ($view_modes as $name => $mode) {
    $options[$name] = $mode['label'];
  }
  
  $element['component_reference_view_mode'] = array(
    '#title' => t('View mode'),
    '#type' => 'select',
    '#options' => $options,
    '#default_value' => $settings['component_reference_view_mode'],
    // Never empty, so no #empty_option
  );
  
  return $element;
}

/**
 * Implements hook_field_formatter_settings_summary().
 */
function components_field_formatter_settings_summary($field, $instance, $view_mode) {
  $display = $instance['display'][$view_mode];
  $settings = $display['settings'];
  //$settings = $display['instance_settings'];
  //$settings = $instance['settings'];
  
  if ($display['type'] === 'component_reference_component') {
    $entity_info = entity_get_info('component');
    $view_modes = $entity_info['view modes'];
    $mode = $view_modes[$settings['component_reference_view_mode']]['label'];
    $summary = t('View mode: %view_mode', array('%view_mode' => $mode));
  }
  else {
    $summary = NULL;
  }
  return $summary;
}

/**
 * Implements hook_field_formatter_prepare_view().
 */
function components_field_formatter_prepare_view($entity_type, $entities, $field, $instances, $langcode, &$items, $displays) {
  // Collect cids to load
  $cids = array();
  foreach ($displays as $id => $display) {
    if ($display['type'] == 'component_reference_component') {
      foreach ($items[$id] as $delta => $item) {
        // Default to 'not accessible'
        $items[$id][$delta]['access'] = FALSE;
        if (!empty($item['cid']) && is_numeric($item['cid'])) {
          $items[$id][$delta]['access'] = TRUE;
          $cids[$item['cid']] = $item['cid'];
        }
      }
    }
  }
  
  $components = component_load_multiple($cids);
  
  // Add the loaded components to the items
  foreach ($displays as $id => $display) {
    if ($display['type'] == 'component_reference_component') {
      foreach ($items[$id] as $delta => $item) {
        if ($item['access'] && isset($components[$item['cid']])) {
          $items[$id][$delta]['component'] = $components[$item['cid']];
        }
      }
    }
  }
}

/**
 * Implements hook_field_formatter_view().
 */
function components_field_formatter_view($entity_type, $entity, $field, $instance, $langcode, $items, $display) {
  $result = array();
  $settings = $display['settings'];
  //$settings = $display['instance_settings'];
  //$settings = $instance['settings'];
  
  switch ($display['type']) {
    case 'component_reference_component':
      // @TODO: do we need $view_mode?
      $view_mode = $settings['component_reference_view_mode'];
      // @TODO: consider to create component_view_multiple() for performance reasons.
      // @see node_reference_field_formatter_view()
      // If we're going to use specific view_modes for each component, this won't be possible...
      foreach ($items as $delta => $item) {
        if ($item['access'] && isset($item['component'])) {
          $result[$delta] = component_view($item['component']);
        }
        else {
          $result[$delta]['#markup'] = t('The component with cid=@cid cannot be found.', array('@cid' => $item['cid']));
        }
      }
      break;
  }
  
  return $result;
}

/**************************/
/* Field Type API: Widget */
/**************************/

/**
 * Implements hook_field_widget_info().
 */
function components_field_widget_info() {
  return array(
    'component_reference_cid' => array( // only for development purposes
      'label' => t('Component ID'),
      'field types' => array('component_reference'),
    ),
    'component_reference_embed' => array(
      'label' => t('Component Embedded'),
      'field types' => array('component_reference'),
    ),
  );
}

/**
 * Implements hook_field_widget_form().
 */
function components_field_widget_form(&$form, &$form_state, $field, $instance, $langcode, $items, $delta, $element) {
  $base = $element;
  
  switch ($instance['widget']['type']) {
    case 'component_reference_cid':
    case '$component_reference_embed':
      $element['cid'] = array(
        '#type' => 'textfield',
        '#title' => t('Component ID'),
        '#default_value' => isset($items[$delta]['cid']) ? $items[$delta]['cid'] : NULL,
      ) + $base; // @TODO: $base?
      break;
    case 'component_reference_embed':
      if (isset($items[$delta]['cid'])) {
        $component = component_load($items[$delta]['cid']);
        
        if ($component) {
          $element['component-' . $component->cid] = array(
            '#type' => 'markup',
            '#markup' => drupal_render(component_view($component)),
          );
          
          $component_links = array(
            'items' => array(),
            'title' => NULL,
            'type' => 'ul',
            'attributes' => array('class' => 'action-links')
          );
          
          $component_links['items'][] = l(
            t('Delete'),
            'component/' . $component->cid . '/delete',
            array('query' => drupal_get_destination())
          );
          
          $element['component-' . $component->cid . '-delete'] = array(
            '#type' => 'markup',
            '#markup' => theme_item_list($component_links),
          );
          
          $element['cid'] = array(
            '#type' => 'hidden',
            '#value' => isset($items[$delta]['cid']) ? $items[$delta]['cid'] : NULL,
          ) + $base; // @TODO: $base?
        }
        else {
          $element['component-' . $items[$delta]['cid'] . '-cannot-be-found'] = array(
            '#type' => 'markup',
            '#markup' => t('The component with cid=@cid cannot be found.', array('@cid' => $items[$delta]['cid'])),
          );
        }
      }
      else {
        $component_types = $field['settings']['referenceable_types'];
        $component_types_info = field_info_bundles('component');
        
        if ($component_types) {
          $entity_type = $instance['entity_type'];
          // @TODO: check if $form_state[$entity_type] ALWAYS works when
          // it should be, for entity type "node" it's ok...
          //
          // Make sure to prevent causing errors related to a call from the
          // Components Reference Field settings form,
          // there won't be a $form_state[$entity_type]
          if (isset($form_state[$entity_type])) {
            list($entity_id) = entity_extract_ids($entity_type, $form_state[$entity_type]);
            
            $component_links = array(
              'items' => array(),
              'title' => NULL,
              'type' => 'ul',
              'attributes' => array('class' => 'action-links')
            );
            
            foreach ($component_types as $type) {
              if (isset($component_types_info[$type])) {
                // @TODO: check access for creating a component of this type
                $component_links['items'][] = l(
                  $component_types_info[$type]['label'],
                  'component/add/' . $type . '/' . $entity_type . '/' . $entity_id . '/' . $field['field_name'],
                  array('query' => drupal_get_destination())
                );
              }
            }
            
            $element['add_more_components'] = array(
              '#type' => 'markup',
              '#markup' => theme_item_list($component_links),
            );
          }
        }
      }
      break;
  }
  
  return $element;
}

/**
 * Implements hook_form_alter().
 */
function components_form_alter(&$form, &$form_state, $form_id) {
  $field_instances = components_get_field_instances();
  
  $field_occurrences = array();
  foreach ($field_instances as $field_instance => $entity_types) {
    foreach ($entity_types as $entity_type => $bundle) {
      if (isset($form['#entity_type']) && isset($form['#bundle'])) {
        if ($form['#entity_type'] == $entity_type && $form['#bundle'] == $bundle) {
          $field_occurrences[] = $field_instance;
        }
      }
    }
  }
  
  // Check if this form has "Component reference"-fields
  if (count($field_occurrences) < 1) {
    return;
  }
  
  foreach ($field_occurrences as $field_instance) {
    if (isset($form[$field_instance])) {
      $entity_type = $form['#entity_type'];
      $bundle = $form['#bundle'];
      $field = &$form[$field_instance];
      
      // Remove the "Add another item"-button from the form
      unset($field[$field['#language']]['add_more']);
    }
  }
}

/**
 * Implements hook_field_widget_error().
 */
function components_field_widget_error($element, $error, $form, $form_state) {
  switch ($error['error']) {
    case 'component_reference_cid_invalid':
      $error_element = $element;
      break;
    default:
      $error_element = $element;
      break;
  }
  form_error($error_element, $error['message']);
}

/**
 * Gets a component
 * 
 * @return
 *   A component object or FALSE.
 */
function components_field_get_entity(&$item, $component_type = NULL) {
  if (isset($item['cid'])) {
    return component_load($item['cid']);
  }
  elseif (!isset($item['entity']) && isset($component_type)) {
    $item['entity'] = component_create(array('type' => $component_type));
  }
  return isset($item['entity']) ? $item['entity'] : FALSE;
}

/*********/
/* OTHER */
/*********/

/**
 * Implements hook_form_FORM_ID_alter().
 */
function components_form_field_ui_field_edit_form_alter(&$form, &$form_state) {
  if ($form['#field']['type'] == 'component_reference') {
    // Remove Default Value Widget because it makes no sense.
    if (isset($form['instance']['default_value_widget'])) {
      unset($form['instance']['default_value_widget']);
    }
    // Set the cardinality of the field settings to UNLIMITED by default.
    if (isset($form['field']['cardinality']['#default_value'])) {
      $form['field']['cardinality']['#default_value'] = FIELD_CARDINALITY_UNLIMITED;
    }
  }
}

/**
 * Return all field instances of the field type "Component reference".
 * 
 * The returned array will be in the form of:
 * array(field_name => array(entity_type => bundle))
 * Example:
 * array(field_components_reference => array(node => page))
 */
function components_get_field_instances() {
  $instances = field_info_instances();
  $field_types = field_info_field_types();
  
  $fields = array();
  
  foreach ($instances as $entity_type => $type_bundles) {
    foreach ($type_bundles as $bundle => $bundle_instances) {
      foreach ($bundle_instances as $field_name => $instance) {
        $field = field_info_field($field_name);
        if ($field['type'] == 'component_reference') {
          $fields[$field_name][$entity_type] = $bundle;
        }
      }
    }
  }
  
  return $fields;
}
