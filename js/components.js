(function ($) {

  Drupal.behaviors.component = {
    attach: function (context, settings) {
      $('input[name="component_type_layout"]').change(function () {
        $('input[name="component_type_layout"]').each(function () {
          if ($(this).is(':checked')) {
            $(this).parent('.form-item-component-type-layout').addClass('selected');
          }
          else {
            $(this).parent('.form-item-component-type-layout').removeClass('selected');
          }
        });
      });
    }
  };
  
  /**
   * Provide the HTML to create the modal dialog.
   */
  Drupal.theme.prototype.ComponentsModal = function () {
    var html = '';

    html += '<div id="components-modal">';
    html += '  <div class="modal-header clearfix">';
    html += '    <span id="modal-title"></span>';
    html += '    <div class="modal-close-wrapper">';
    html += '      <a href="#" class="close">';
    html += '        <span class="element-invisible">' + Drupal.CTools.Modal.currentSettings.closeText + '</span>';
    html += '      </a>';
    html += '    </div>';
    html += '  </div>';
    html += '  <div class="ctools-modal-content">';
    html += '    <div id="modal-content" class="modal-content"></div>';
    html += '  </div>';
    html += '</div>';

    return html;
  }

})(jQuery);
