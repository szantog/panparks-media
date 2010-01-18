Drupal.media = Drupal.media || {};

(function ($) {

Drupal.behaviors.mediaFormatForm = {
  attach: function (context, settings) {
    // Create the jQuery UI dialog box to handle all dialog events.
    $("#media-format-form").bind('submit', Drupal.media.mediaFormatSelected);
  }
};

Drupal.media.mediaFormatSelected = function (e) {
  var formatType = $("input[@name='format']:checked", this).val();
  var formattedMedia = {type: formatType, html: Drupal.settings.media.formatFormFormats[formatType]};
  $(this).parents('#dialog').trigger('mediaSelected', [formattedMedia]);
  $(this).parents('#dialog').dialog('close');
  return false;
}

})(jQuery);
