Drupal.media = Drupal.media || {};

(function ($) {
  
  Drupal.wysiwyg.plugins.media = {
    /**
     * Execute the button.
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
     */
    mediaBrowserOnSelect: function (mediaFiles, instanceId) {
      var mediaFile = mediaFiles[0];
      var options = {};
      Drupal.media.popups.mediaStyleSelector(mediaFile, function (formattedMedia) {
        Drupal.wysiwyg.plugins.media.insertMediaFile(mediaFile, formattedMedia.type, formattedMedia.html, Drupal.wysiwyg.instances[instanceId]);
      }, options);
      
      return;
    },

    styleChooserOnLoad: function (e) {
      var options = e.data;
      debug.debug("Loaded the stylechooser");
      debug.debug(options);
      // $("reference to stylechooser body").load("media format form");
      // bind to the submit button
    },

    insertMediaFile: function(mediaFile, viewMode, formattedMedia, wysiwygInstance) {
      // Hack to allow for use of .html()
      var embeddedMedia = $('<div>' + formattedMedia + '</div>');
      // add the fid attribute to the image
      $('img', embeddedMedia).attr('fid', mediaFile.fid);
      $('img', embeddedMedia).attr('view_mode', viewMode);
      tagContent = Drupal.wysiwyg.plugins.media.createTag(embeddedMedia);
      // When tagmap is defined such as node/edit, block/configure
      if(Drupal.settings.tagmap) {
        Drupal.settings.tagmap[tagContent] = Drupal.wysiwyg.plugins.media.addWrapper(embeddedMedia.html());
      }
      // When tagmap is not defined such as node/add/, block/add
      else {
        Drupal.settings.tagmap = { };
        Drupal.settings.tagmap[tagContent] = Drupal.wysiwyg.plugins.media.addWrapper(embeddedMedia.html());
      }
      wysiwygInstance.insert(Drupal.wysiwyg.plugins.media.addWrapper(embeddedMedia.html()) + "&nbsp;");
    },
    
    /**
     * Atach function, called when a rich text editor loads.
     * This finds all [[tags]] and replaces them with the html
     * that needs to show in the editor.
     * 
     */
    attach: function(content, settings, instanceId) {
      matches = content.match(/\[\[.*?\]\]/g);
      tagmap = Drupal.settings.tagmap;
      if(matches) {
        for (i=0; i<matches.length; i++) {
          for (var tagContent in tagmap ) {
            if (tagContent === matches[i]) {
              // This probably needs some work...
              // We need to somehow get the fid propogated here.
              // We really want to
              matches[i] = matches[i].replace('[[','');
              matches[i] = matches[i].replace(']]','');
              mediaObj = JSON.parse(matches[i]);
              imgMarkup = $(tagmap[tagContent]);
              $('img',imgMarkup).attr('fid',mediaObj.fid);
              $('img',imgMarkup).attr('view_mode',mediaObj.view_mode);
              content = content.replace(tagContent,this.addWrapper(imgMarkup.html()));
            }
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
      $('div.media-embedded',content).each(function (elem) {
        tagContent = Drupal.wysiwyg.plugins.media.createTag(this);
        $(this).replaceWith(tagContent);
      });
      return content.html();
    },
    
    addWrapper: function(htmlContent) {
      return '<div class="media-embedded">' + htmlContent + '</div>';  
    },
    
    createTag: function(mediaObj) {
        imgNode = $("img", mediaObj);
        // Collect all attribs to be stashed into tagContent
        attribs = {};
        imgAttribList = imgNode[0].attributes;
        for(i=0; i<imgAttribList.length; i++) {
          attribs[imgAttribList[i].nodeName] = imgAttribList[i].nodeValue; 
        }
        // Remove elements from attribs using the blacklist
        for(var blackList in Drupal.settings.media.blacklist) {
          delete attribs[Drupal.settings.media.blacklist[blackList]];
        }
        tagContent = {
          "type": 'media',
          //@todo: This will be selected from the format form
          "view_mode": imgNode.attr('view_mode'),
          "fid" : imgNode.attr('fid'),
          "attributes": attribs
        };
        return '[[' + JSON.stringify(tagContent) + ']]';
      }
  };
})(jQuery);
