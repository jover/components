(function ($) {
  
  /**
   * Implementation of Drupal.behaviors for components group.
   */
  Drupal.behaviors.components_group = {
    attach: function (context, settings) {
      Drupal.behaviors.components_group.addPlaceholders();
      if (settings.components_group.launchSortMode) {
        Drupal.behaviors.components_group.enterSortMode();
      }
    },
    
    addPlaceholders: function () {
      $('div.components-group .components').each(function () {
        var empty_text = '';
        // If the components group is empty
        if ($('div.component', this).length == 0) {
          empty_text = Drupal.settings.components_group.emptyComponentsGroupText;
          // We need a placeholder
          if ($('.placeholder', this).length == 0) {
            $(this).append('<div class="placeholder"></div>');
          }
          $('.placeholder', this).html(empty_text);
        }
        else {
          $('.placeholder', this).remove();
        }
      });
    },
    
    /**
     * Enter "sort" mode.
     */
    enterSortMode: function () {
      Drupal.behaviors.components_group.addPlaceholders();
      Drupal.behaviors.components_group.setupDrawer();
    },
    
    /**
     * Exit "sort" mode.
     */
    exitSortMode: function () {
      Drupal.behaviors.components_group.addPlaceholders();
    },
    
    /**
     * Helper for enterOrderMode; sets up drag-and-drop
     */
    setupDrawer: function () {
      // Initialize drag-and-drop.
      var components_groups = $('div.components-group .components');
      components_groups.sortable({
        connectWith: components_groups,
        cursor: 'move',
        cursorAt: {top: 0},
        dropOnEmpty: true,
        items: '> div.component',
        placeholder: 'component-placeholder clearfix',
        tolerance: 'pointer',
        start: Drupal.behaviors.components_group.start,
        over: Drupal.behaviors.components_group.over,
        sort: Drupal.behaviors.components_group.sort,
        update: Drupal.behaviors.components_group.update
      });
    },
    
    /**
     * While dragging, make the component appear as a disabled component.
     *
     * This function is called on the jQuery UI Sortable "start" event.
     *
     * @param event
     *  The event that triggered this callback.
     * @param ui
     *  An object containing information about the item that is being dragged.
     */
    start: function (event, ui) {
      var item = $(ui.item);
      
      // If the component is already in disabled state, don't do anything.
      if (!item.hasClass('component-disabled')) {
        item.css({height: 'auto'});
      }
    },
    
    /**
     * While dragging, adapt block's width to the width of the region it is moved
     * into.
     *
     * This function is called on the jQuery UI Sortable "over" event.
     *
     * @param event
     *  The event that triggered this callback.
     * @param ui
     *  An object containing information about the item that is being dragged.
     */
    over: function (event, ui) {
      var item = $(ui.item);
      
      // If the component is in disabled state, remove width.
      item.css({width: $(this).width()});
    },
    
    /**
     * While dragging, adapt the component's position to stay connected with the
     * position of the mouse pointer.
     *
     * This function is called on the jQuery UI Sortable "sort" event.
     *
     * @param event
     *  The event that triggered this callback.
     * @param ui
     *  An object containing information about the item that is being dragged.
     */
    sort: function (event, ui) {
      var item = $(ui.item);
      
      if (event.pageX > ui.offset.left + item.width()) {
        item.css({left: event.pageX});
      }
    },
    
    update: function (event, ui) {
      
    }
  };
  
})(jQuery);
