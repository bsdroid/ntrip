
var map;
var marker;

function initialize(lat, lon) {

  var myOptions = {
    center:     new google.maps.LatLng(lat, lon),
    zoom:       17,
    mapTypeId:  google.maps.MapTypeId.SATELLITE,
    panControl: true
  };

  map    = new google.maps.Map(document.getElementById("map_canvas"), myOptions);

  marker = new google.maps.Marker({
    map:       map,
    position:  new google.maps.LatLng(lat, lon),
    //  icon: jsonObj.icon
  });
}

function gotoLocation(lat, lon) {
  var position = new google.maps.LatLng(lat, lon);
  map.setCenter(position);
  marker.setPosition(position);
}

