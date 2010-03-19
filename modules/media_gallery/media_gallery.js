// $Id$

(function ($) {
  Drupal.behaviors.experimentalMediaBrowser = {
    attach: function (context) {
      $('.media-widget-closed', context).once('mediaWidgetDrawrerDone', this.processWidget);
    },

    processWidget: function() {
      var $this = $(this);
      var $wrapper = $('.gallery-fields-wrapper', this);
      $wrapper.slideUp();
        $('.edit-description-link', this).bind('click', function() {
          $wrapper.slideDown();
          $this.addClass('media-widget-open');
          $this.removeClass('media-widget-closed');
        });
    }
  };
})(jQuery);