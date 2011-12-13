<div id="<?php print $id; ?>" class="<?php print implode(' ', $classes_array); ?>">
  <?php if ($components): ?>
    <div class="components">
      <?php foreach ($components as $component): ?>
        <?php print render($component); ?>
      <?php endforeach; ?>
    </div>
  <?php endif; ?>
</div>
