
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
    map:      map,
    position: new google.maps.LatLng(lat, lon),
    title:    "X",
    //    animation: google.maps.Animation.DROP,
    //    icon: jsonObj.icon
  });

  //google.maps.event.addListener(map, "click", function() {} )
}

function gotoLocation(lat, lon) {
  var position = new google.maps.LatLng(lat, lon);
  map.setCenter(position);
  marker.setPosition(position);
}

// function appendMarker(jsonObj)
// {
//   var marker = new google.maps.Marker({
//     position: new google.maps.LatLng( jsonObj.geometry.location.lat, jsonObj.geometry.location.lng ),
//     map: map,
//     title: jsonObj.name,
//     animation: google.maps.Animation.DROP,
//     icon: jsonObj.icon
//   });
// 
//   google.maps.event.addListener(marker, 'click', function() {
//     QtPlaces.markerClicked( jsonObj['reference'] )
//   });
// 
//   markers.push(marker)
// }
// 
// function gotoPlace(json, zoom)
// {
//   map.setCenter( new google.maps.LatLng( json.geometry.location.lat, json.geometry.location.lng ) )
//   map.setZoom(zoom)
// }

