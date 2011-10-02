/**
 * @file
 * Component Popup (jQuery Dialog)
 */
(function ($) {
  
  Drupal.components = {};
  
  Drupal.ajax.prototype.commands.componentsAddComponentDialog = function (ajax, response, status) {
    var id = response.id;
    var popup_title = Drupal.settings.components.popup.title;
    var popup_body = Drupal.settings.components.popup.id;
    var popup_popup = Drupal.settings.components.popup.popup;
    $(popup_title).html(response.title);
    $(popup_body).html(response.output);
    $(popup_popup).dialog('open');
    $('#' + id).remove();
    Drupal.components.resizeModal();
  };
  
  Drupal.behaviors.componentsPopup = {
    attach: function (context, settings) {
      if (!settings.components) {
        return;
      }
      
      // Create a jQuery UI dialog, but leave it closed
      var dialog_popup = $(settings.components.popup.popup, context);
      dialog_popup.dialog({
        'autoOpen': false,
        'dialogClass': 'components-popup-dialog',
        'modal': true,
        'position': 'center',
        'resizable': false,
        'width': 750
      });
    }
  };
  
  /**
   * @see Drupal.viewsUi.resizeModal
   */
  Drupal.components.resizeModal = function (e, no_shrink) {
    var $ = jQuery;
    var $modal = $('.components-popup-dialog');
    var $scroll = $('.scroll', $modal);
    if ($modal.size() == 0 || $modal.css('display') == 'none') {
      return;
    }
  
    var maxWidth = parseInt($(window).width() * .85); // 70% of window
    var minWidth = parseInt($(window).width() * .6); // 70% of window
  
    // Set the modal to the minwidth so that our width calculation of
    // children works.
    $modal.css('width', minWidth);
    var width = minWidth;
  
    // Don't let the window get more than 80% of the display high.
    var maxHeight = parseInt($(window).height() * .8);
    var minHeight = 200;
    if (no_shrink) {
      minHeight = $modal.height();
    }
  
    if (minHeight > maxHeight) {
      minHeight = maxHeight;
    }
  
    var height = 0;
  
    // Calculate the height of the 'scroll' region.
    var scrollHeight = 0;
  
    scrollHeight += parseInt($scroll.css('padding-top'));
    scrollHeight += parseInt($scroll.css('padding-bottom'));
  
    $scroll.children().each(function() {
      var w = $(this).innerWidth();
      if (w > width) {
        width = w;
      }
      scrollHeight += $(this).outerHeight(true);
    });
  
    // Now, calculate what the difference between the scroll and the modal
    // will be.
  
    var difference = 0;
    difference += parseInt($scroll.css('padding-top'));
    difference += parseInt($scroll.css('padding-bottom'));
    difference += $('#components-popup-title').outerHeight(true);
    difference += $('.form-actions', $modal).outerHeight(true);
  
    height = scrollHeight + difference;
  
    if (height > maxHeight) {
      height = maxHeight;
      scrollHeight = maxHeight - difference;
    }
    else if (height < minHeight) {
      height = minHeight;
      scrollHeight = minHeight - difference;
    }
  
    if (width > maxWidth) {
      width = maxWidth;
    }
  
    // Get where we should move content to
    var top = ($(window).height() / 2) - (height / 2);
    var left = ($(window).width() / 2) - (width / 2);
  
    $modal.css({
      'top': top + 'px',
      'left': left + 'px',
      'width': width + 'px',
      'height': height + 'px'
    });
  
    // Ensure inner popup height matches.
    $(Drupal.settings.views.ajax.popup).css('height', height + 'px');
  
    $scroll.css({
      'height': scrollHeight + 'px',
      'max-height': scrollHeight + 'px'
    });
  
  };
  
})(jQuery);
