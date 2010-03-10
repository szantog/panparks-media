/**
 * Creates a namespace.
 * @return
 */
function namespace() {
    var a=arguments, o=null, i, j, d;
    for (i=0; i<a.length; i=i+1) {
        d=a[i].split(".");
        o=window;
        for (j=0; j<d.length; j=j+1) {
            o[d[j]]=o[d[j]] || {};
            o=o[d[j]];
        }
    }
    return o;
};


/**
 * Stolen from jQuery 1.4
 */
jQuery.recursiveParam = function ( a ) {

  var s = [],
    param_traditional = jQuery.param.traditional;

  function add( key, value ){
    // If value is a function, invoke it and return its value
    value = jQuery.isFunction(value) ? value() : value;
    s[ s.length ] = encodeURIComponent(key) + '=' + encodeURIComponent(value);
  }

  // If an array was passed in, assume that it is an array
  // of form elements
  if ( jQuery.isArray(a) || a.jquery )
    // Serialize the form elements
    jQuery.each( a, function() {
      add( this.name, this.value );
    });

  else
    // Encode parameters from object, recursively. If
    // jQuery.param.traditional is set, encode the "old" way
    // (the way 1.3.2 or older did it)
    jQuery.each( a, function buildParams( prefix, obj ) {

      if ( jQuery.isArray(obj) )
        jQuery.each( obj, function(i,v){
          // Due to rails' limited request param syntax, numeric array
          // indices are not supported. To avoid serialization ambiguity
          // issues, serialized arrays can only contain scalar values. php
          // does not have this issue, but we should go with the lowest
          // common denominator
          add( prefix + ( param_traditional ? "" : "[]" ), v );
        });

      else if ( typeof obj == "object" )
        if ( param_traditional )
          add( prefix, obj );

        else
          jQuery.each( obj, function(k,v){
            buildParams( prefix ? prefix + "[" + k + "]" : k, v );
          });

      else
        add( prefix, obj );

    });

  // Return the resulting serialization
  return s.join("&").replace('%20', "+");
}