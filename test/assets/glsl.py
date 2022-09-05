import sys, os

glslangValidator_name = 'glslangValidator'
if sys.platform == 'win32':
    glslangValidator_name = 'glslangValidator.exe'


def glsl_cvt(filename):
    cmd = "%s -H -V %s -o %s.spv > %s.spv.txt" % (glslangValidator_name, filename, filename, filename)
    os.system(cmd)


if __name__ == "__main__":
    glsl_cvt(sys.argv[1])