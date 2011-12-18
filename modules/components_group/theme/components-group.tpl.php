<div id="<?php print $id; ?>" class="<?php print implode(' ', $classes_array); ?>">
  <?php if ($contextual_links): ?>
    <?php print render($contextual_links); ?>
  <?php endif; ?>
  <?php if ($components): ?>
    <div class="components">
      <?php foreach ($components as $component): ?>
        <?php print render($component); ?>
      <?php endforeach; ?>
    </div>
  <?php endif; ?>
</div>
