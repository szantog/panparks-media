namespace('Drupal.media');
Drupal.media.mediaFormatSelected = {};
(function ($) {
Drupal.behaviors.mediaFormatForm = {
  attach: function (context, settings) {
    //Setup the value to start with.
    
    $("input[name='format']", context).change(Drupal.media.mediaFormatSelected);
    $("input[name='format']:checked").trigger('change');
  }
};

Drupal.media.mediaFormatSelected = function (e) {
  var formatType = $(this).val();
  var formattedMedia = {type: formatType, html: Drupal.settings.media.formatFormFormats[formatType]};
  debug.debug(formattedMedia);
  Drupal.media.formattedMedia = formattedMedia;
}

})(jQuery);
