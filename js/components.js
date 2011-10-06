/**
 * Provide the HTML to create the modal dialog.
 */
Drupal.theme.prototype.ComponentsModal = function () {
  var html = '';
  
  html += '<div id="components-modal">';
  html += '  <div class="components-modal-content">';
  html += '    <div class="modal-header">';
  html += '      <h2 id="modal-title"></h2>';
  html += '      <a href="#" class="close">' + Drupal.CTools.Modal.currentSettings.closeText + '</a>';
  html += '    </div>';
  html += '    <div id="modal-content"></div>';
  html += '  </div>';
  html += '</div>';
  
  return html;
}
