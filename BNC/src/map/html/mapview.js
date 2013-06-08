var map;
var markers= [];
var QtPlaces;

var searchCircle;

function initialize(x, y)
{
    var myOptions = {
        center: new google.maps.LatLng(x, y),
        zoom: 17,
        mapTypeId: google.maps.MapTypeId.SATELLITE,
        panControl: true
    };
    map = new google.maps.Map(document.getElementById("map_canvas"), myOptions)
    //google.maps.event.addListener(map, "click", function() {} )
}

function getCurrentPointOfView() {
    return map.getBounds().getCenter().toUrlValue(6);
}

function removeMarkers()
{
    while(markers.length)
        markers.pop().setMap(null)
}

// function appendMarker(jsonObj)
// {
//     var marker = new google.maps.Marker({
//         position: new google.maps.LatLng( jsonObj.geometry.location.lat, jsonObj.geometry.location.lng ),
//         map: map,
//         title: jsonObj.name,
//         animation: google.maps.Animation.DROP,
//         icon: jsonObj.icon
//     });
// 
//     google.maps.event.addListener(marker, 'click', function() {
//         QtPlaces.markerClicked( jsonObj['reference'] )
//     });
// 
//     markers.push(marker)
// }
// 
// function gotoPlace(json, zoom)
// {
//     map.setCenter( new google.maps.LatLng( json.geometry.location.lat, json.geometry.location.lng ) )
//     map.setZoom(zoom)
// }

function gotoLocation( lat, lng, zoom )
{
    map.setCenter( new google.maps.LatLng( lat, lng ) )
    map.setZoom(zoom)
}

function recreateSearchCircle(lat, lng, radius)
{
    var searchCircleOptions = {
      strokeColor: "#000000",
      strokeOpacity: 0.75,
      strokeWeight: 1,
      fillColor: "#000000",
      fillOpacity: 0.35,
      map: map,
      center: new google.maps.LatLng( lat, lng ),
      radius: radius
    };
    searchCircle = new google.maps.Circle(searchCircleOptions);
}
