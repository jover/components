<div id="<?php print $id; ?>" class="<?php print implode(' ', $classes_array); ?>">
  <?php if ($components_group_add_link): ?>
    <div class"components-group-add-link">
      <?php print render($components_group_add_link); ?>
    </div>
  <?php endif; ?>
  <?php if ($components): ?>
    <div class="components">
      <?php foreach ($components as $component): ?>
        <?php print render($component); ?>
      <?php endforeach; ?>
    </div>
  <?php endif; ?>
</div>
