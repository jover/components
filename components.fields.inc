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
  return (empty($item['cid'])) ? TRUE : FALSE;
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
      $element['cid'] = array(
        '#type' => 'textfield',
        '#title' => t('Component ID'),
        '#default_value' => isset($items[$delta]['cid']) ? $items[$delta]['cid'] : NULL,
      ) + $base; // @TODO: $base?
      break;
    case 'component_reference_embed':
      $field_item_name = $field['field_name'] . '-' . $langcode . '-' . $delta;
      $field_item_container_id = drupal_html_id($field_item_name . '-container');
      $field_item_cid_id = drupal_html_id($field_item_name . '-cid');
      
      if (isset($items[$delta]['cid'])) {
        if ($component = component_load($items[$delta]['cid'])) {
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
            //'#type' => 'hidden',
            '#type' => 'textfield',
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
        if ($component_types = $field['settings']['referenceable_types']) {
          $component_types_info = field_info_bundles('component');
          
          $element['component-container'] = array(
            '#type' => 'container',
            '#id' => $field_item_container_id,
          );
            
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
                //'component/add/' . $type . '/' . $entity_type . '/' . $entity_id . '/' . $field['field_name'],
                //array('query' => drupal_get_destination())
                'component/add/' . $type . '/' . $field_item_container_id . '/' . $field_item_cid_id . '/nojs/',
                array('attributes' => array('class' => 'use-ajax'))
              );
            }
          }
          
          $element['component-container']['components_links'] = array(
            '#type' => 'markup',
            '#markup' => theme_item_list($component_links),
          );
          
          $element['cid'] = array(
            //'#type' => 'hidden',
            '#type' => 'textfield',
            '#value' => NULL,
            '#prefix' => '<div id="' . $field_item_cid_id . '">',
            '#suffix' => '</div>',
          ) + $base; // @TODO: $base?
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
      $language = $field['#language'];
      
      // Remove the "Add another item"-button from the form
      //unset($field[$language]['add_more']);
      $field[$language]['add_more_url'] = array(
        '#type' => 'hidden',
        // The name of the class is the #id of $field[$language]['add_more'] with "-url" suffix.
        '#attributes' => array('class' => array('component-add-url')),
        '#value' => url('component/add/' . $entity_type . '/1/' . $field[$language]['#field_name'] . '/nojs'),
      );
      
      unset($field[$language]['add_more']['#ajax']);
      unset($field[$language]['add_more']['#submit']);
      $field[$language]['add_more']['#type'] = 'button';
      $field[$language]['add_more']['#id'] = 'component-add';
      $field[$language]['add_more']['#value'] = t('Add component');
      $field[$language]['add_more']['#attributes']['class'][] = 'ctools-use-modal';
      
      // Do not show an extra empty field
      $max_delta = $field[$language]['#max_delta'];
      if ($max_delta > 0) {
        unset($field[$language][$max_delta]);
        $field[$language]['#max_delta'] = $max_delta - 1;
      }
    }
  }
  
  // Include the CTools tools that we need.
  ctools_include('ajax');
  ctools_include('modal');
  // Add CTools' javascript to the page.
  ctools_modal_add_js();
  
  /*$form['#attached']['library'][] = array('system', 'ui.dialog');
  $form['#attached']['library'][] = array('system', 'drupal.ajax');
  $form['#attached']['library'][] = array('system', 'jquery.form');
  
  $form['#attached']['css'] = components_get_admin_css();
  
  $form['#attached']['js'][] = drupal_get_path('module', 'components') . '/js/components-popup.js';
  $form['#attached']['js'][] = array(
    'data' => array('components' => array('popup' => array(
      'id' => '#components-popup-body',
      'title' => '#components-popup-title',
      'popup' => '#components-popup',
    ))),
    'type' => 'setting',
  );
  
  $form['components-popup'] = array(
    //'#theme_wrappers' => array('components_popup'),
    '#theme' => 'components_popup',
    '#weight' => 9999,
  );*/
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

/**
 * Create an array of Views admin CSS for adding or attaching.
 *
 * This returns an array of arrays. Each array represents a single
 * file. The array format is:
 * - file: The fully qualified name of the file to send to drupal_add_css
 * - options: An array of options to pass to drupal_add_css.
 * 
 * @see views_ui_get_admin_css()
 */
function components_get_admin_css() {
  $module_path = drupal_get_path('module', 'components');
  $list = array();
  $list[$module_path . '/css/components-admin.css'] = array();
  $list[$module_path . '/css/components-admin.theme.css'] = array();
  
  // Add in any theme specific CSS files we have
  $themes = list_themes();
  $theme_key = $GLOBALS['theme'];
  while ($theme_key) {
    $css_file = $module_path . '/css/components-admin.' . $theme_key . '.css';
    if (file_exists($css_file)) {
      $list[$css_file] = array(
        'group' => CSS_THEME,
      );
    }
    $theme_key = isset($themes[$theme_key]->base_theme) ? $themes[$theme_key]->base_theme : '';
  }
  
  return $list;
}
