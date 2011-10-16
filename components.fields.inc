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
      'settings' => array(),
      'instance_settings' => array(
        'referenceable_types' => array(),
      ),
      'default_widget' => 'component_builder',
      'default_formatter' => 'component_reference_component',
      // Add entity property info
      'property_type' => 'component',
    ),
  );
}

/**
 * Implements hook_field_instance_settings_form().
 */
function components_field_instance_settings_form($field, $instance) {
  $settings = $instance['settings'];
  
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
        'error' => 'component_builder_invalid',
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
      'description' => t('Display the referenced component'),
      'field types' => array('component_reference'),
      'settings' => array(),
    ),
  );
}

/**
 * Implements hook_field_formatter_settings_form().
 */
function DEPRECATED_components_field_formatter_settings_form($field, $instance, $view_mode, $form, $form_state) {
  $display = $instance['display'][$view_mode];
  $settings = $display['settings'];
  
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
function DEPRECATED_components_field_formatter_settings_summary($field, $instance, $view_mode) {
  $display = $instance['display'][$view_mode];
  $settings = $display['settings'];
  
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
  
  switch ($display['type']) {
    case 'component_reference_component':
      foreach ($items as $delta => $item) {
        if ($item['access'] && isset($item['component'])) {
          // If view mode is set, render the component in this view mode.
          if (isset($item['view_mode']) && !is_null($item['view_mode'])) {
            $result[$delta] = component_view($item['component'], $item['view_mode']);
          }
          // If view mode is not set, render the component in default view mode.
          else {
            $result[$delta] = component_view($item['component']);
          }
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
    'component_builder' => array(
      'label' => t('Component Builder'),
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
      );
          
      $element['view_mode'] = array(
        '#type' => 'hidden',
        '#value' => isset($items[$delta]['view_mode']) ? $items[$delta]['view_mode'] : NULL,
      ) + $base; // @TODO: $base?
      break;
    case 'component_builder':
      if (isset($items[$delta]['cid'])) {
        if ($component = component_load($items[$delta]['cid'])) {
          ctools_include('ajax');
          ctools_include('modal');
          
          if (isset($items[$delta]['view_mode']) && !is_null($items[$delta]['view_mode'])) {
            $element['component-' . $component->cid] = array(
              '#type' => 'markup',
              '#markup' => drupal_render(component_view($component, $items[$delta]['view_mode'])),
            );
          }
          else {
            $element['component-' . $component->cid] = array(
              '#type' => 'markup',
              '#markup' => drupal_render(component_view($component)),
            );
          }
          
          $component_links = array(
            'items' => array(),
            'title' => NULL,
            'type' => 'ul',
            'attributes' => array('class' => 'action-links')
          );
          
          $component_links['items'][] = ctools_modal_text_button(t('Edit'), 'component/' . $component->cid . '/edit/nojs', t('Edit'), 'ctools-modal-components-style');
          $component_links['items'][] = ctools_modal_text_button(t('Delete'), 'component/' . $component->cid . '/delete/nojs', t('Delete'), 'ctools-modal-components-style');
          
          $element['component-' . $component->cid . '-action-links'] = array(
            '#type' => 'markup',
            '#markup' => theme_item_list($component_links),
          );
          
          $element['cid'] = array(
            '#type' => 'hidden',
            '#value' => isset($items[$delta]['cid']) ? $items[$delta]['cid'] : NULL,
          );
          
          $element['view_mode'] = array(
            '#type' => 'hidden',
            '#value' => isset($items[$delta]['view_mode']) ? $items[$delta]['view_mode'] : NULL,
          ) + $base; // @TODO: $base?
        }
        else {
          $element['component-' . $items[$delta]['cid'] . '-cannot-be-found'] = array(
            '#type' => 'markup',
            '#markup' => t('The component with cid=@cid cannot be found.', array('@cid' => $items[$delta]['cid'])),
          );
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
  if (!isset($form['#entity_type']) || !isset($form_state[$form['#entity_type']])) {
    return;
  }
  
  $entity_type = $form['#entity_type'];
  $entity = $form_state[$entity_type];
  
  list($entity_id) = entity_extract_ids($entity_type, $entity);
  
  $field_instances = components_get_field_instances();
  
  $field_occurrences = array();
  foreach ($field_instances as $field_instance => $entity_types) {
    foreach ($entity_types as $type => $bundle) {
      if (isset($entity_type) && isset($form['#bundle'])) {
        if ($entity_type == $type && $form['#bundle'] == $bundle) {
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
      $field = &$form[$field_instance];
      $language = $field['#language'];
      
      if (is_null($entity_id)) {
        // Components cannot be added to the host entity if the host entity is new (empty entity id).
        $field = array(
          '#type' => 'markup',
          '#markup' => t('Components can be added only after the host entity is saved.'),
        );
      }
      else {
        unset($field[$language]['add_more']);
        
        $field[$language]['add_more']['url'] = array(
          '#type' => 'hidden',
          // The name of the class is the #id of $field[$language]['add_more']['button] with "-url" suffix.
          '#attributes' => array('class' => array('component-add-url')),
          '#value' => url('component/add/' . $entity_type . '/' . $entity_id . '/' . $field[$language]['#field_name'] . '/nojs'),
        );
        
        $field[$language]['add_more']['button'] = array(
          '#type' => 'button',
          '#value' => t('Add component'),
          '#id' => 'component-add',
          '#attributes' => array('class' => array(
            'ctools-use-modal',
            'ctools-modal-components-style',
          )),
        );
        
        // Do not show an extra empty field
        $max_delta = $field[$language]['#max_delta'];
        if ($max_delta > 0) {
          unset($field[$language][$max_delta]);
          $field[$language]['#max_delta'] = $max_delta - 1;
        }
      }
    }
  }
  
  // Include the CTools tools that we need.
  ctools_include('ajax');
  ctools_include('modal');
  
  // Add CTools' javascript to the page.
  ctools_modal_add_js();
  
  // Create our own javascript that will be used to theme a modal.
  $modal_style = array(
    'components-style' => array(
      'modalSize' => array(
        'type' => 'scale',
        'width' => .8,
        'height' => .8,
        'addWidth' => 0,
        'addHeight' => 0,
        'contentRight' => 25,
        'contentBottom' => 45,
      ),
      'modalOptions' => array(
        'opacity' => .8,
        'background-color' => '#000',
      ),
      'animation' => 'fadeIn',
      'modalTheme' => 'ComponentsModal',
      'throbber' => theme('image', array('path' => ctools_image_path('ajax-loader.gif', 'ctools_ajax_sample'), 'alt' => t('Loading...'), 'title' => t('Loading'))),
    ),
  );

  drupal_add_js($modal_style, 'setting');
  
  ctools_add_js('components', 'components');
  ctools_add_css('components', 'components');
}

/**
 * Implements hook_field_widget_error().
 */
function components_field_widget_error($element, $error, $form, $form_state) {
  switch ($error['error']) {
    case 'component_builder_invalid':
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
