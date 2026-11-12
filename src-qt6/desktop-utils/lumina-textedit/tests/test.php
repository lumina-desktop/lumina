<?php

class Mail
{
    private $from = ['name' => '', 'email' => ''];
    private $to = [];
    private $subject = '';
    private $message = '';
    private $files = [];
    private $multipart = false;
    private $boundary = '';
    private $uniqId = '';
    private $replyTo = [];
    private $timestamp = null;

    const CRLF = "\r\n";


    public function __construct()
    {
        $this->uniqId = '<php-mail-' . md5(microtime()) . mt_rand() . '@git.php.net>';
    }

    /**
     * Return unique id of mail
     * @return string unique Id of message in format: '<php-mail-...@git.php.net';
     */
    public function getId()
    {
        return $this->uniqId;
    }

    /**
     * Add parent mail for this mail
     * @param string $uniqId unique Id of message in format: '<php-mail-...@git.php.net';
     */
    public function addReplyTo($uniqId)
    {
        $this->replyTo[] = $uniqId;
    }

    /**
     * Add attached text file to mail
     * @param string $name unique file name
     * @param string $data file content
     */
    public function addTextFile($name , $data)
    {
        $this->files[trim($name)] = chunk_split(base64_encode($data), 76, self::CRLF);
    }

    /**
     * Return length of attached file
     * @param string $name unique file name
     * @return int file length
     */
    public function getFileLength($name)
    {
        $name = trim($name);
        return isset($this->files[$name]) ? strlen($this->files[$name]) : 0;
    }

    /**
     * Delete attached file
     * @param string $name unique file name
     */
    public function dropFile($name)
    {
        $name = trim($name);
        unset($this->files[$name]);
    }

    /**
     * Set "From" address
     * @param string $email email author address
     * @param string $name author name
     */
    public function setFrom($email, $name = '')
    {
        $this->from = ['email' => trim($email), 'name' => trim($name)];
    }

    /**
     * Add recipient address
     * @param string $email recipient address
     * @param string $name recipient name
     */
    public function addTo($email, $name = '')
    {
        $this->to[] = ['email' => trim($email), 'name' => trim($name)];
    }

    /**
     * Set mail subject
     * @param string $subject subject
     */
    public function setSubject($subject)
    {
        $this->subject = trim($subject);
    }

    /**
     * Set timestamp
     * @param string $timestamp timestamp
     */
    public function setTimestamp($timestamp)
    {
        $this->timestamp = trim($timestamp);
    }

    /**
     * Set mail body text
     * @param string $message body text
     */
    public function setMessage($message)
    {
        $this->message = $message;
    }


    /**
     * Format header string
     * @param string $name header name
     * @param string $value header value
     * @return string header string
     */
    private function makeHeader($name, $value)
    {
        return $name . ': ' . $value;
    }

    /**
     * Format address string
     * @param array $address array with email adress and name
     * @return string address string
     */
    private function makeAddress(array $address)
    {
        return $address['name'] ? $this->utf8SafeEncode($address['name'], 100) . ' <'. $address['email']  . '>' : $address['email'];
    }

    /**
     * Cut end encode string by mb_encode_mimeheader
     * @param string $value utf8 string
     * @param int $maxLenght max length
     * @return string encoded string
     */
    private function utf8SafeEncode($value, $maxLenght = null)
    {
        if ($maxLenght) $value = mb_substr($value, 0, $maxLenght);
        return mb_encode_mimeheader($value, 'UTF-8', 'Q');
    }

    /**
     * Prepare heade part of mail
     * @return string header part of mail
     */
    private function makeHeaders()
    {
        $headers = [];
        $headers[] = $this->makeHeader('From', $this->makeAddress($this->from));
        $headers[] = $this->makeHeader('Message-ID', $this->uniqId);
        if (count($this->replyTo)) {
            $replyTo = implode(' ', $this->replyTo);
            $headers[] = $this->makeHeader('References', $replyTo);
            $headers[] = $this->makeHeader('In-Reply-To', $replyTo);
        }
        $headers[] = $this->makeHeader('MIME-Version', '1.0');
        $headers[] = $this->makeHeader('Date', date(DATE_RFC2822, $this->timestamp ?: time()));
        if ($this->multipart) {
            $this->boundary = sha1($this->uniqId);
            $headers[] = $this->makeHeader('Content-Type', 'multipart/mixed; boundary="' . $this->boundary . '"');
        } else {
            $headers[] = $this->makeHeader('Content-Type', 'text/plain; charset="utf-8"');
            // we use base64 for avoiding some problems sush string length limit, safety encoding etc.
            $headers[] = $this->makeHeader('Content-Transfer-Encoding', 'quoted-printable');
        }
        return implode(self::CRLF , $headers);
    }

    /**
     * Prepare body part of mail
     * @return string mail body
     */
    private function makeBody()
    {
        $body = '';
        if ($this->multipart) {
                $body .= '--' . $this->boundary . self::CRLF;
                $body .= $this->makeHeader('Content-Type', 'text/plain; charset="utf-8"') . self::CRLF;
                $body .= $this->makeHeader('Content-Transfer-Encoding', 'quoted-printable') . self::CRLF;
                $body .= self::CRLF;
                $body .= quoted_printable_encode($this->message);
            foreach ($this->files as $name => $data) {
                $body .= self::CRLF . '--' . $this->boundary . self::CRLF;
                $body .= $this->makeHeader('Content-Type', 'text/plain; charset="utf-8"') . self::CRLF;
                $body .= $this->makeHeader('Content-Transfer-Encoding', 'base64') . self::CRLF;
                $body .= $this->makeHeader('Content-Disposition', 'attachment; filename="' . $name . '"') . self::CRLF;
                $body .= self::CRLF;
                $body .= $data;
            }
            $body .= self::CRLF . '--' . $this->boundary . '--';
        } else {
            $body = quoted_printable_encode($this->message);
        }
        return $body;
    }

    /**
     * Send current mail
     * @return bool
     */
    public function send()
    {
        $this->multipart = (bool) count($this->files);

        $receivers = implode(', ', array_map([$this, 'makeAddress'], $this->to));
        $subject = $this->utf8SafeEncode($this->subject, 450);
        $headers = $this->makeHeaders();
        $body = $this->makeBody();

        return mail($receivers, $subject, $body, $headers, "-f noreply@php.net");
    }
}
