<?php

class AlertSender {

    public static function SendAlert() {
        $email = filter_input(INPUT_GET, "email");
        $messageData = filter_input(INPUT_GET, "message");
        $key = filter_input(INPUT_GET, "key");

        if ($key != 1) {
            die();
        }
        $message = "Tombo Alert Trigger Detected.\r\n";
        $message.= urldecode($messageData);

        $to = $email;

        $subject = "Tombo Security Risk Alert";

        $enter = "\r\n";
        $headers = "From: donotreply@costaricamakers.com $enter";
        $headers .= "MIME-Version: 1.0 $enter";
        $headers .= "Content-type: text/plain; charset=utf-8 $enter";

        $result = mail($to, $subject, $message, $headers);
        print_r($result);
    }
}

AlertSender::SendAlert();