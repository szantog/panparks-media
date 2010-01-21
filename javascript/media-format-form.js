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
    // When we select a new radio, then load it up to be sent to the WYSIWYG.
    $("input[name='format']", context).change(Drupal.media.formatForm.setFormValues);
    //Setup the value to start with.
    $("input[name='format']:checked").trigger('change');
  }
};


Drupal.media.formatForm.getOptions = function() {
  var ret = {};
  $('input, select, textarea', $('#edit-options')).each(function() {
    //@todo: this isn't working... why?
    //ret[this.name] = $(this).fieldValue()[0];
    ret[this.name] = $(this).val();
  });
  return ret;
}

Drupal.media.formatForm.setFormValues = function() {
  var formatType = $("input[name='format']:checked").val();
  
  Drupal.media.formattedMedia = {type: formatType, options: Drupal.media.formatForm.getOptions(), html: Drupal.settings.media.formatFormFormats[formatType]};
}

})(jQuery);
