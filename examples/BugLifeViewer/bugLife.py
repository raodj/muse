import direct.directbase.DirectStart
from pandac.PandaModules import *

from direct.task import Task
from direct.actor import Actor
from direct.interval.IntervalGlobal import *
import math

#Load the first environment model
environ = loader.loadModel("models/environment")
environ.reparentTo(render)
environ.setScale(1,1,1)
environ.setPos(-8,42,0)

#Task to move the camera
def SpinCameraTask(task):
  angledegrees = task.time * 6.0
  angleradians = angledegrees * (math.pi / 180.0)
  base.camera.setPos(20*math.sin(angleradians),-20.0*math.cos(angleradians),3)
  base.camera.setHpr(angledegrees, 0, 0)
  return Task.cont

#taskMgr.add(SpinCameraTask, "SpinCameraTask")

#Load the panda actor, and loop its animation
#for x in range(0,10):
pandaActor = Actor.Actor("models/panda-model",{"walk":"models/panda-walk4"})
pandaActor.setScale(0.003,0.003,0.003)
#pandaActor.setPos(0+x,0,0)
pandaActor.reparentTo(render)
pandaActor.loop("walk")


#Create the four lerp intervals needed to walk back and forth
pandaPosInterval1= pandaActor.posInterval(13,Point3(0,-10,0), startPos=Point3(0,10,0))
pandaPosInterval2= pandaActor.posInterval(13,Point3(0,10,0), startPos=Point3(0,-10,0))
pandaHprInterval1= pandaActor.hprInterval(3,Point3(180,0,0), startHpr=Point3(0,0,0))
pandaHprInterval2= pandaActor.hprInterval(3,Point3(0,0,0), startHpr=Point3(180,0,0))

#Create and play the sequence that coordinates the intervals
pandaPace = Sequence(pandaPosInterval1, pandaHprInterval1,
  pandaPosInterval2, pandaHprInterval2, name = "pandaPace")
pandaPace.loop()


#Run the tutorial
run() 