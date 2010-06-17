// $Id$

/**
 *  @file
 *  Attach behaviors to formatter radio select when selecting a media's display
 *  formatter.
 */

(function ($) {
namespace('Drupal.media.formatForm');

Drupal.media.mediaFormatSelected = {};

Drupal.behaviors.mediaFormatForm = {
  attach: function (context, settings) {
    
  }
};

Drupal.media.formatForm.getOptions = function () {
  // Get all the values
  var ret = {}; $.each($('#media-format-form fieldset#edit-options *').serializeArray(), function (i, field) { ret[field.name] = field.value; });
  return ret;
};

Drupal.media.formatForm.getFormattedMedia = function () {
  var formatType = $("select#edit-format option:selected").val();
  return { type: formatType, options: Drupal.media.formatForm.getOptions(), html: Drupal.settings.media.formatFormFormats[formatType] };
};

})(jQuery);
