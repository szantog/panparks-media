// $Id$

/**
 *  @file
 *  Attach behaviors to formatter radio select when selecting a media's display
 *  formatter.
 */

// Set the namespace.
namespace('Drupal.media.formatForm');

Drupal.media.mediaFormatSelected = {};

(function ($) {
Drupal.behaviors.mediaFormatForm = {
  attach: function (context, settings) {
    $('#format-group-wrapper fieldset').hide();
    $('<a href="#">' + Drupal.t('Change') + '</a>')
      .bind('click', function(e) {
        $('#format-group-wrapper fieldset').show();
        $('#format-group-wrapper fieldset').removeClass('collapsed');
        $('#format-description').remove();
      })
      .appendTo('#format-description');
  }
};


Drupal.media.formatForm.getOptions = function() {
  // Get all the values
  var ret = {}; $.each($('#media-format-form fieldset#edit-options *').serializeArray(), function(i, field) { ret[field.name] = field.value; });
  return ret;
}

Drupal.media.formatForm.getFormattedMedia = function() {
  var formatType = $("input[name='format']:checked").val();
  return {type: formatType, options: Drupal.media.formatForm.getOptions(), html: Drupal.settings.media.formatFormFormats[formatType]};
}

})(jQuery);
