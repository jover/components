<?php

/**
 * @file
 * Default theme implementation for components.
 *
 * Available variables:
 * - $content: An array of comment items. Use render($content) to print them all, or
 *   print a subset such as render($content['field_example']). Use
 *   hide($content['field_example']) to temporarily suppress the printing of a
 *   given element.
 * - $classes: String of classes that can be used to style contextually through
 *   CSS. It can be manipulated through the variable $classes_array from
 *   preprocess functions. The default values can be one or more of the
 *   following:
 *   - component: The current template type, i.e., "theming hook".
 *   - component-[type]: The current component type. For example, if the component is a
 *     "Header component" it would result in "component-header". Note that the machine
 *     name will often be in a short form of the human readable label.
 *   - component-unpublished: Unpublished components visible only to administrators.
 *
 * Other variables:
 * - $component: Full component object. Contains data that may not be safe.
 * - $type: component type, i.e. header, paragraph, etc.
 * - $classes_array: Array of html class attribute values. It is flattened
 *   into a string within the variable $classes.
 * 
 * Component status variables:
 * - $view_mode: View mode, e.g. 'default', ...
 * - $status: Flag for published status.
 * 
 * Field variables: for each field instance attached to the component a corresponding
 * variable is defined, e.g. $component->body becomes $body. When needing to access
 * a field's raw values, developers/themers are strongly encouraged to use these
 * variables. Otherwise they will have to explicitly specify the desired field
 * language, e.g. $component->body['en'], thus overriding any language negotiation
 * rule that was previously applied.
 *
 * @see template_preprocess()
 * @see template_preprocess_entity()
 * @see template_process()
 */
?>
<div id="component-<?php print $component->cid; ?>" class="<?php print $classes; ?> clearfix"<?php print $attributes; ?>>
  <?php if ($contextual_links): ?>
    <?php print render($contextual_links); ?>
  <?php endif; ?>
  <?php print render($content); ?>
</div>
