from distutils.core import setup

print("Running setup program")

setup(name='PyScstudio',
      version='1.1.0',
      description='Python Scstudio Objects',
      author='Boris Ranto',
      author_email='borix60@gmail.net',
      url='http://scstudio.sourceforge.net/',
      packages=['pysc', 'pycheck', 'pyscuser'],
      package_dir={'pysc': 'src/data/pysc', 'pycheck': 'src/check/pycheck', 'pyscuser': 'src/check/pyscuser'},
     )
