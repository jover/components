<?php

/**
 * @file
 * Provides Entity metadata integration.
 */

/**
 * Extend the defaults.
 */
class ComponentsMetadataController extends EntityDefaultMetadataController {
  
  public function entityPropertyInfo() {
    $info = parent::entityPropertyInfo();
    $properties = &$info[$this->type]['properties'];
    
    $properties['label'] = array(
      'name' => t('Label'),
      'description' => t('The component label.'),
      'getter callback' => 'entity_property_getter_method',
    );
    
    $properties['type'] = array(
      'type' => 'component_type',
      'getter callback' => 'entity_property_getter_method',
      'setter callback' => 'entity_property_verbatim_set',
      'setter permission' => 'administer components',
      'required' => TRUE,
      'description' => t('The component type.'),
    ) + $properties['type'];
  
    return $info;
  }
}

