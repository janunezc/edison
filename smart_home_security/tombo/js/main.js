/**
 * Function to be called by the system when everything is ready @ browser. 
 */
$(document).ready(function () {
    configureEvents();
});

/**
 * Configures behavior of each actionable control in the page.
 * @returns {undefined}
 */
function configureEvents() {
    $("#btnBegin").click(function () {
        $("#btnBegin").hide();
        $("#btnStop").show();
        stopFlag = false;
        beginCapture();
    }).removeAttr("disabled").text("BEGIN");

    $("#btnStop").click(function () {
        $("#iconwelcome").css("color", "gray");
        stopFlag = true;
        $("#btnBegin").show();
        $("#btnStop").hide();
        $("#imgClock").hide();
    });
}

/**
 * Flag used to control the recurrent retrieval of information from pvCloud
 * @type Boolean
 */
var stopFlag = false;

/**
 * Initiates and performs the recurrent capture of data from pvCloud
 * @returns {undefined}
 */
function beginCapture() {
    $("#imgClock").hide();
    if (!stopFlag) { /** /si stopflag es false, o sea si el boton no se ha presionado */
        var time = 10;/** /txtCheckTime es el tiempo que uno le da para calcular*/

        $("#created_datetime").html("loading...");

        GetPVCLOUDValue(function (response) { /** /Obtiene el valor de pvCloud, ejecuta hasta que GetPVCLOUDValue tenga un valor */
            console.log(response);
            processResponse(response); /** /Data = response */
            $("#imgClock").show();
            setTimeout(beginCapture, time * 1000); /** /Se setea solo para volver a contar cada tiempo segun el #txtCheckTime */
        });
        $("#iconwelcome").css("color", "green");
    }
}

/**
 * Processes a response value coming from retrieving data from pvCloud
 * @param {object} response
 * @returns {undefined}
 */
function processResponse(response) {
    var value = JSON.parse(response.vse_value); /** /  Pasa de tipo JSON (String) a un objeto Java. {T:20,H:60} --> value.H value.T   */
    var created_datetime = response.created_datetime; /** /    created_datetime = atributo de pvCloud q se da como un string */
    if (value) {
        /** /    Pasando los valores a la pagina */
        console.log("VALUE!!!!");
        console.log(value.per_status);
        if (value.per_status === "A") {
            console.log("Painting icon to RED...");
            console.log($("#iconPerimeter").css("color", "red"));
            console.log("I should be red");
        } else {
            $("#iconPerimeter").css("color", "green");
        }

        if (value.front_door_status === "L") {
            console.log("Painting icon to RED...");
            console.log($("#iconFrontDoor").css("color", "red"));
            console.log("I should be red");
        } else {
            $("#iconFrontDoor").css("color", "green");
        }

        if (value.z1_status === "L") {
            console.log("Painting icon to RED...");
            console.log($("#iconZ1").css("color", "red"));
            console.log("I should be red");
        } else {
            $("#iconZ1").css("color", "green");
        }

        if (value.z2_status === "L") {
            console.log("Painting icon to RED...");
            console.log($("#iconZ2").css("color", "red"));
            console.log("I should be red");
        } else {
            $("#iconZ2").css("color", "green");
        }
        if (value.temp_status === "L") {
            console.log("Painting icon to RED...");
            console.log($("#iconTemp").css("color", "red"));
            console.log("I should be red");
        } else {
            $("#iconTemp").css("color", "green");
        }
        if (value.light_status === "L") {
            console.log("Painting icon to RED...");
            console.log($("#iconLight").css("color", "red"));
            console.log("I should be red");
        } else {
            $("#iconLight").css("color", "green");
        }

        if (value.house_status === "U") {
            console.log("Painting icon to RED...");
            console.log($("#iconHouse").css("color", "red"));
            console.log("I should be red");
            processWarnings("U");
        } else {
            $("#iconHouse").css("color", "green");
        }

        var now = new Date();
        var createdDateObj = new Date(created_datetime); /** /  pasa  created_datetime de string a date */
        createdDateObj.setHours(createdDateObj.getHours() + 1); /** / Hora de creacion del registro */
        console.log(now);

        console.log(createdDateObj);
        $("#created_datetime").html(created_datetime); /** / Asigna a #created_datetime el valor html de createdDateObj */

        var diffsec = (now - createdDateObj) / 1000;

        if (diffsec > 60) {
            $("#created_datetime").css("color", "red"); /** / No ha vuelto a recibir datos hace 60 seg, se pone en rojo */
        } else {
            $("#created_datetime").css("color", "green");
        }

    }
    console.log(value);
}

var messageSent = false;
var messageSentEmail = "";
function processWarnings(alarm_status) {

    var message = "";
    var email = $("#txtAlertEmail").val();

    if (alarm_status === "U") {

        message = " ";
    }

    if (message !== "" && messageSentEmail !== email && email !== "") {
        var url = "sendAlert.php?email=" + email + "&key=1&message=" + encodeURIComponent(message);
        console.log("SENDING EMAIL...");
        console.log(url);
        $.ajax({
            url: url,
            success: function (response) {
                messageSentEmail = email;
                console.log("EMAIL SENT");
            }
        });
    }

}


/**
 * Actually pefroms a Web Service Call to pvCloud to retrieve last value registered for the app.
 * @param {type} callback
 * @returns {undefined}
 */
function GetPVCLOUDValue(callback) { /** /   Llamada en el browser, todo en el main.js */
    var url = BuildPVCloudURL_GetLastValue();

    $.ajax(url, {/** /Utiliza el url y hace que el browser llame a PVCloud, asincronica, no se detiene ahi, sigue llamando*/
        success: function (data) { /** /Data son los datos de PVCloud */
            console.log(data);
            callback(data); /** / Cuando tiene el dato, hace el callback y devuelve la info GetPVCLOUDValue, si no hay valores data es null  */
        },
        error: function (error) { /** / Error de comunicacion  */
            console.log(error);
            return null;
        }

    });

}

/**
 * Actually pefroms a Web Service Call to pvCloud to retrieve last value registered for the app.
 * @param {type} callback
 * @returns {undefined}
 */
function GetPVCLOUDValue(callback) { /** /   Llamada en el browser, todo en el main.js */
    var url = BuildPVCloudURL_GetLastValue();

    $.ajax(url, {/** /Utiliza el url y hace que el browser llame a PVCloud, asincronica, no se detiene ahi, sigue llamando*/
        success: function (data) { /** /Data son los datos de PVCloud */
            callback(data); /** / Cuando tiene el dato, hace el callback y devuelve la info GetPVCLOUDValue, si no hay valores data es null  */
        },
        error: function (error) { /** / Error de comunicacion  */
            console.log("ERROR OCCURRED");
            callback(null);
            console.log(error);
            return null;
        }

    });

}

/**
 * Crafts a URL with the necessary parameters for getting last value from pvCloud
 * @returns {String}
 */
function BuildPVCloudURL_GetLastValue() {
    //Keeping Api Key and connection authentication data @ Server Side
    return "ws_GetPVCloudData.php";
}