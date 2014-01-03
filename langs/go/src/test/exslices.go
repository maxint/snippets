package main

import "tour/pic"

func Pic(dx, dy int) [][]uint8 {
	var img = make([][]uint8, 256)
	for y := 0; y < 256; y++ {
		img[y] = make([]uint8, 256)
		for x := 0; x < 256; x++ {
			img[y][x] = uint8(x + y)
		}
	}
	return img
}

func main() {
	pic.Show(Pic)
}
