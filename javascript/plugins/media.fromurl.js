(function ($) {
  namespace('Drupal.media.browser.plugin');
  
  Drupal.media.browser.plugin.fromurl = function(mediaBrowser, options) {
    return {
      /* Abstract */
      init: function() {
        tabset = mediaBrowser.getTabset();
        tabset.tabs('add', '#fromurl', 'From URL');
        mediaBrowser.listen('tabs.tabSelected', function (e, id) {
          if (id == 'fromurl') {
            mediaBrowser.getContentArea().load(Drupal.settings.basePath + 'media/add/from_url?destination=' + window.location.href +' form#media-add-from-url');
          }
        });
      }
    }
  };
  
  // For now, I guess self registration makes sense.
  // Really though, we should be doing it via drupal_add_js and some settings
  // from the drupal variable.
  //@todo: needs a review.
  Drupal.media.browser.register('fromurl', Drupal.media.browser.plugin.fromurl, {});
})(jQuery);