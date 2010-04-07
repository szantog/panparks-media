// $Id$

/**
 * @file
 * JavaScript for the Media types administrative form.
 */

(function ($) {

/**
 * Thumbnails display
 */
Drupal.behaviors.mediaTypesAdmin = {
  attach: function (context) {
    // Toggle the 'other' text field on Match type.
    if ($('.form-item-match-type input:checked').val() != '0') {
      $('.form-item-match-type-other').hide();
    }
    $('.form-item-match-type input').change(function() {
      if ($(this).val() == '0') {
        $('.form-item-match-type-other').slideDown('fast');
      }
      else {
        $('.form-item-match-type-other').slideUp('fast');
      }
    });
  }
};

})(jQuery);
