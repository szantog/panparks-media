// $Id$

(function ($) {
  Drupal.behaviors.experimentalMediaBrowser = {
    attach: function (context) {
      // On the edit form.
      $('.media-widget-closed', context).once('mediaWidgetDrawrerDone', this.processWidget);

      // Temporary jCycle code (needs to be abstracted, with settings, etc)

      $('.jCycle-container').once('jcycleActivated', function () {
        $(this).cycle({
          fx: 'fade',
          timeout: 6000,
          delay: -2000
        }
        );
      });
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