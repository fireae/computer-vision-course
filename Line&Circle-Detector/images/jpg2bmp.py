import PIL.Image as Image

def jpg2bmp(name, inputPath, outputPath):
    inFilePath = inputPath + '/' + name + '.jpg'
    outFilePath = outputPath + '/' + name + '.bmp'
    im = Image.open(inFilePath, 'r')

    # Resize
    # scaleR = 300/im.size[0]
    # im = im.resize((int(scaleR*im.size[0]), int(scaleR*im.size[1])))
    # print(im.size)

    im.save(outFilePath)

for i in range(6):
    jpg2bmp(str(i+1), './Dataset1', './Dataset1/bmp')
    jpg2bmp(str(i+1), './Dataset2', './Dataset2/bmp')
