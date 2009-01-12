<?php

/**
 * public:// stream wrapper class.
 */
class DrupalStreamWrapperPublic extends DrupalStreamWrapper {

  // A handle to the file opened by stream_open().
  private $pathKey = 'stream_public_path';
  private $pathDefault = 'sites/default/files';

  /**
   * interpolate the url for a public stream into a
   * real stream or path.
   */
  function interpolateUrl($url) {
    $basepath = variable_get($this->pathKey, $this->pathDefault);
    
    // just in case stream_public_path is s3://, ftp://, etc. Don't call PHP's
    // realpath().
    if (parse_url($basepath, PHP_URL_SCHEME)) {
      $path =  $basepath . parse_url($url, PHP_URL_PATH);;
    }
    else {
    // interpolate relative paths for basepath, and strip relative paths from 
    // url path.
      $path = realpath($basepath) . str_replace('/..','', parse_url($url, PHP_URL_PATH));
    }
    return $path;
  }
}


