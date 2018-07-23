import PIL.Image as Image

def jpg2bmp(name, format,  inputPath, outputPath):
    inFilePath = inputPath + '/' + name + format
    outFilePath = outputPath + '/' + name + '.bmp'
    im = Image.open(inFilePath, 'r')

    # Resize
    # scaleR = 300/im.size[0]
    # im = im.resize((int(scaleR*im.size[0]), int(scaleR*im.size[1])))
    # print(im.size)

    im.save(outFilePath)

for i in range(6):
    jpg2bmp(str(i+1), '.jpg', './Dataset1', './Dataset1/bmp')
    if i < 2: 
        jpg2bmp(str(i+1), '.PNG', './Dataset2', './Dataset2/bmp')
