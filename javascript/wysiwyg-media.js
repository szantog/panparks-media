Drupal.wysiwyg.plugins.media = {

  /**
   * Execute the button.
   */
  invoke: function(data, settings, instanceId) {
    if (data.format == 'html') {
      options = {};
      jQuery().mediaBrowser( function (mediaFiles, options) {
    	  // Currently we can select only one file from the browser
    	  mediaFile = mediaFiles[0];
    	  // Insert the preview of the file returned into the editor.
    	  Drupal.wysiwyg.instances[instanceId].insert(mediaFile.preview);
      	},options);
    }
    
  },
  
  /**
   * Atach function, called when a rich text editor loads
   */
  attach: function(content, settings, instanceId) {
	  matches = content.match(/\[\[.*?\]\]/g);
	  tagmap = Drupal.settings.tagmap;
	  if(matches) {
	  for (i=0; i< matches.length; i++) {
		  for (var tagContent in tagmap ) {
			  if (tagContent === matches[i]) {
				content = content.replace(matches[i],tagmap[tagContent]);
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
    var $content = jQuery('<div>' + content + '</div>');
    jQuery('a.media-thumbnail',$content).each(function (elem){
      var imgNode = jQuery("img",this);
    	var $foo = {
    		"type": 'media',
    		"view_mode": 'thumbnail',
    		"fid" : imgNode.attr('fid'),
    		"attributes": { 
    			"width" : imgNode.attr('width'),
    			"height" : imgNode.attr('height'),
    	  }
    	}
    	debug.debug(jQuery("img.media-img-thumbnail").attr('width'));
    	$foo = '[[' + JSON.stringify($foo) + ']]';
    	jQuery(this).replaceWith($foo);
    });
    return $content.html();
  },
};
