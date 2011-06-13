<?php

/**
 * @file
 * Provides Entity metadate integration.
 */

/**
 * Extend the defaults.
 */
class EntityBuildingBlocksMetadataController extends EntityDefaultMetadataController {
  
  public function entityPropertyInfo() {
    $info = parent::entityPropertyInfo();
    $properties = &$info[$this->type]['properties'];
    
    $properties['label'] = array(
      'name' => t('Label'),
      'description' => t('The Entity Building Block label.'),
      'getter callback' => 'entity_property_getter_method',
    );
    
    $properties['type'] = array(
      'type' => 'ebb_type',
      'getter callback' => 'entity_property_getter_method',
      'setter callback' => 'entity_property_verbatim_set',
      'setter permission' => 'administer ebbs',
      'required' => TRUE,
      'description' => t('The Entity Building Block Type.'),
    ) + $properties['type'];
  
    return $info;
  }
}

