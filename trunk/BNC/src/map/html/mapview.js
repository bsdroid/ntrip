
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

  var image = new google.maps.MarkerImage("qrc:///map/html/crosshair.png",
                                          null, 
                                          null,
                                          new google.maps.Point(20, 20),
                                          new google.maps.Size(40,40)
                                         );
  marker = new google.maps.Marker({
    map:      map,
    position: new google.maps.LatLng(lat, lon),
    icon:     image,
  });
}

function gotoLocation(lat, lon) {
  var position = new google.maps.LatLng(lat, lon);
  map.setCenter(position);
  marker.setPosition(position);
}

