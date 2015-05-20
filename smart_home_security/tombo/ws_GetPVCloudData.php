<?php
error_reporting(E_ERROR);

class WebServiceClass {

    /**
     * 
     * @return be_session
     */
    public static function GetPVCloudData() {

        $url = "http://costaricamakers.com/pvcloud_backend/vse_get_value_last.php?account_id=1&app_id=33&api_key=440707b410e99a5506af6cf2752633e0d8888f63&optional_label=HOUSE_STATUS";
        $result = file_get_contents($url);
        
        return $result;
    }

}

$result = WebServiceClass::GetPVCloudData();

header("Access-Control-Allow-Origin: *");
header('Content-Type: application/json');
echo $result;