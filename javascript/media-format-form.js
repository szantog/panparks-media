// $Id$

/**
 *  @file
 *  Attach behaviors to formatter radio select when selecting a media's display
 *  formatter.
 */

// Set the namespace.
namespace('Drupal.media');

Drupal.media.mediaFormatSelected = {};

(function ($) {
Drupal.behaviors.mediaFormatForm = {
  attach: function (context, settings) {
    // When we select a new radio, then load it up to be sent to the WYSIWYG.
    $("input[name='format']", context).change(Drupal.media.mediaFormatSelected);

    //Setup the value to start with.
    $("input[name='format']:checked").trigger('change');
  }
};

Drupal.media.mediaFormatSelected = function (e) {
  // What's the display formatter we've selected?
  var formatType = $(this).val();

  // Set the HTML to be sent to the WYSIWYG browser.
  var formattedMedia = {type: formatType, html: Drupal.settings.media.formatFormFormats[formatType]};
  debug.debug(formattedMedia);
  Drupal.media.formattedMedia = formattedMedia;
}

})(jQuery);
