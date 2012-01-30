<div id="<?php print $id; ?>" class="<?php print implode(' ', $classes_array); ?>">
  <?php if ($label): ?>
    <h2 class="components-group-label"><?php print $label; ?></h2>
  <?php endif; ?>
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
