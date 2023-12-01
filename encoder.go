package main

import (
	"crypto/rc4"
	"encoding/hex"
	"fmt"
	"io/ioutil"
	"math/rand"
	"os"
	"strings"
	"time"
)

func g() string {
	rand.Seed(time.Now().UnixNano())
	chars := "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
	length := rand.Intn(10) + 16
	var key = strings.Builder{}
	for i := 0; i < length; i++ {
		key.WriteByte(chars[rand.Intn(len(chars))])
	}
	return key.String()
}

func reverseString(str string) string {
	runes := []rune(str)
	for i, j := 0, len(runes)-1; i < j; i, j = i+1, j-1 {
		runes[i], runes[j] = runes[j], runes[i]
	}
	return string(runes)
}

func main() {
	key := g()
	fmt.Println("密钥：", key)

	fileName := reverseString(key) + ".tmp"
	fmt.Println("文件名：", fileName)

	file, err := os.Open("1.bin")
	if err != nil {
		panic(err)
	}

	defer file.Close()

	content, err := ioutil.ReadAll(file)
	if err != nil {
		panic(err)
	}

	xordMessage := make([]byte, len(content))
	for i := 0; i < len(content); i++ {
		xordMessage[i] = content[i] ^ 0xff
	}

	cipher, _ := rc4.NewCipher([]byte(key))
	rc4Message := make([]byte, len(xordMessage))
	cipher.XORKeyStream(rc4Message, xordMessage)

	hexCiphertext := make([]byte, hex.EncodedLen(len(rc4Message)))
	n := hex.Encode(hexCiphertext, rc4Message)
	hexCiphertext = hexCiphertext[:n]

	err = ioutil.WriteFile(fileName, hexCiphertext, 0644)
	if err != nil {
		panic(err)
	}
	fmt.println("完成！")
}
