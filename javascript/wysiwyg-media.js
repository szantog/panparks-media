// $Id$

/**
 *  @file
 *  Attach Media WYSIWYG behaviors.
 */

// Define the namespace.
Drupal.media = Drupal.media || {};

// jQuery safe call.
(function ($) {

  // Define the behavior.
  Drupal.wysiwyg.plugins.media = {
    /**
     * Execute the button.
     * @TODO: Debug calls from this are never called. What's its function?
     */
    invoke: function (data, settings, instanceId) {
      if (data.format == 'html') {
        Drupal.media.popups.mediaBrowser(function (mediaFiles) {
          Drupal.wysiwyg.plugins.media.mediaBrowserOnSelect(mediaFiles, instanceId);
        });
      }
    },

    /**
     * Respond to the mediaBrowser's onSelect event.
     * @TODO: Debug calls from this are never called. What's its function?
     */
    mediaBrowserOnSelect: function (mediaFiles, instanceId) {
      var mediaFile = mediaFiles[0];
      var options = {};
      Drupal.media.popups.mediaStyleSelector(mediaFile, function (formattedMedia) {
        debug.debug(formattedMedia);
        Drupal.wysiwyg.plugins.media.insertMediaFile(mediaFile, formattedMedia.type, formattedMedia.html, formattedMedia.options, Drupal.wysiwyg.instances[instanceId]);
      }, options);

      return;
    },

    insertMediaFile: function(mediaFile, viewMode, formattedMedia, options, wysiwygInstance) {

      if(typeof Drupal.settings.tagmap == 'undefined') {
        Drupal.settings.tagmap = { };
      }
      // @TODO: the folks @ ckeditor have told us that there is no way
      // to reliably add wrapper divs via normal HTML.
      // There is some method of adding a "fake element"
      // But until then, we're just going to embed to img.
      // This is pretty hacked for now.
      //
      var imgElement = $(this.stripDivs(formattedMedia));
      this.addImageAttributes(imgElement, mediaFile.fid, viewMode, options);

      var toInsert = this.outerHTML(imgElement);
      // Create an inline tag
      var inlineTag = Drupal.wysiwyg.plugins.media.createTag(imgElement);
      // Add it to the tag map in case the user switches input formats
      Drupal.settings.tagmap[inlineTag] = toInsert;
      wysiwygInstance.insert(toInsert);
    },

    /**
     * Gets the HTML content of an element
     *
     * @param jQuery element
     */
    outerHTML: function(element) {
      return $('<div>').append( element.eq(0).clone() ).html();
    },

    addImageAttributes: function(imgElement, fid, view_mode, additional) {
      imgElement.attr('fid', fid);
      imgElement.attr('view_mode', view_mode);
      if (additional) {
        for (k in additional) {
          imgElement.attr(k, additional[k]);
        }
      }
      // Class so we can find this image later.
      imgElement.addClass('media-image');
    },

    /**
     * Due to problems handling wrapping divs in ckeditor, this is needed.
     *
     * Going forward, if we don't care about supporting other editors
     * we can use the fakeobjects plugin to ckeditor to provide cleaner
     * transparency between what Drupal will output <div class="field..."><img></div>
     * instead of just <img>, for now though, we're going to remove all the stuff surrounding the images.
     *
     * @param String formattedMedia
     *  Element containing the image
     *
     * @return HTML of <img> tag inside formattedMedia
     */
    stripDivs: function(formattedMedia) {
      return $('<div>').append( $('img', $(formattedMedia)).eq(0).clone() ).html();
    },

    /**
     * Attach function, called when a rich text editor loads.
     * This finds all [[tags]] and replaces them with the html
     * that needs to show in the editor.
     *
     */
    attach: function(content, settings, instanceId) {
      var matches = content.match(/\[\[.*?\]\]/g);
      var tagmap = Drupal.settings.tagmap;
      if(matches) {
        var inlineTag = "";
        for (i = 0; i < matches.length; i++) {
          inlineTag = matches[i];
          if (tagmap[inlineTag]) {
            // This probably needs some work...
            // We need to somehow get the fid propogated here.
            // We really want to
            var tagContent = tagmap[inlineTag];
            var mediaMarkup = this.stripDivs(tagContent); // THis is <div>..<img>

            var _tag = inlineTag;
            _tag = _tag.replace('[[','');
            _tag = _tag.replace(']]','');
            mediaObj = JSON.parse(_tag);

            var imgElement = $(mediaMarkup);
            this.addImageAttributes(imgElement, mediaObj.fid, mediaObj.view_mode);
            var toInsert = this.outerHTML(imgElement);
            content = content.replace(inlineTag, toInsert);
          } else {
            debug.debug("Could not find content for " + inlineTag);
          }
        }
      }
      return content;
    },

    /**
     * Detach function, called when a rich text editor detaches
     */
    detach: function(content, settings, instanceId) {
      var content = $('<div>' + content + '</div>');
      $('img.media-image',content).each(function (elem) {
        tagContent = Drupal.wysiwyg.plugins.media.createTag($(this));
        $(this).replaceWith(tagContent);
      });
      return content.html();
    },

    /**
     * @param jQuery imgNode
     *  Image node to create tag from
     */
    createTag: function(imgNode) {
      // Currently this is the <img> itself
      // Collect all attribs to be stashed into tagContent
      var mediaAttributes = {};

      var imgElement = imgNode[0];

      //@todo: this does not work in IE, width and height are always 0.
      for (i=0; i< imgElement.attributes.length; i++) {
        var attr = imgElement.attributes[i];
        if (attr.specified == true) {
          mediaAttributes[attr.name] = attr.value;
        }
      }
      // Remove elements from attribs using the blacklist
      for(var blackList in Drupal.settings.media.blacklist) {
        delete mediaAttributes[Drupal.settings.media.blacklist[blackList]];
      }
      tagContent = {
        "type": 'media',
        //@todo: This will be selected from the format form
        "view_mode": imgNode.attr('view_mode'),
        "fid" : imgNode.attr('fid'),
        "attributes": mediaAttributes
      };
      return '[[' + JSON.stringify(tagContent) + ']]';
    }
  };
})(jQuery);
