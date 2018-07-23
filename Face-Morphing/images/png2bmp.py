import PIL.Image as Image

def png2bmp(name, format,  inputPath, outputPath):
    inFilePath = inputPath + '/' + name + format
    outFilePath = outputPath + '/' + name + '.bmp'
    im = Image.open(inFilePath, 'r')

    # Resize
    # scaleR = 300/im.size[0]
    # im = im.resize((int(scaleR*im.size[0]), int(scaleR*im.size[1])))
    # print(im.size)
    # im.convert(colors=24)
    
    im.save(outFilePath)

n = 5
for i in range(n):
    png2bmp(str(i+1), '.png', './Dataset2', './Dataset2/bmp')
