#!/usr/bin/env python3
"""ZI-E CAD v0.3 Prototype Release
Parametric top-down packaging/mechanical CAD built from the project's approved design decisions.
Units: mm. Coordinate convention: X left/right, Y front(-)/rear(+), Z up.

This is a complete prototype CAD release, not a production freeze: exact vendor mounting drawings,
printer-specific fits, cable bend radii, measured masses, and physical test results remain verification gates.
"""
from __future__ import annotations
import cadquery as cq
from cadquery import exporters
from pathlib import Path
import math, json, csv

ROOT = Path(__file__).resolve().parents[1]
CAD = ROOT/'cad'; STL = ROOT/'stl'; DATA = ROOT/'data'; DOCS = ROOT/'docs'
for p in [CAD,STL,DATA,DOCS]: p.mkdir(parents=True, exist_ok=True)

P = {
    # Manufacturing / fit policy for prototype FDM
    'wall_cosmetic': 2.4,
    'wall_structural': 3.2,
    'moving_clearance': 0.60,   # per interface nominal prototype starting point
    'static_clearance': 0.30,
    'service_gap': 0.50,
    'cable_clearance': 1.5,
    'm3_clearance': 3.4,
    'm3_insert_pilot': 4.1,

    # Overall packaging generated from component envelopes
    'base_w': 184.0, 'base_d': 142.0, 'base_h': 58.0,
    'torso_h': 128.0, 'torso_bottom_w': 178.0, 'torso_bottom_d': 136.0,
    'torso_top_w': 158.0, 'torso_top_d': 118.0,
    'body_z0': 54.0,

    # Head / head bay
    'head_w': 116.0, 'head_d': 38.0, 'head_h': 82.0,
    'head_bay_w': 123.0, 'head_bay_d': 46.0,
    'head_lift_stroke': 100.0,
    'head_active_bottom': 200.0,
    'head_sleep_bottom': 165.0,
    'head_shutdown_bottom': 100.0,
    'pan_range_deg': 120.0,
    'tilt_up_deg': 45.0, 'tilt_down_deg': 30.0,

    # Display candidate in landscape orientation from ElectroPeak listing
    'display_w': 84.52, 'display_h': 55.26, 'display_t': 4.0,
    'display_active_w': 73.44, 'display_active_h': 48.96,
    'camera_env': [26.0, 16.0, 18.0],
    # Dedicated forward laser rangefinder. Prototype uses Adafruit VL53L1X carrier; final head PCB may use bare sensor.
    'lidar_proto_env': [25.5, 17.5, 4.6],
    'lidar_final_env': [8.0, 6.0, 4.0],
    'lidar_x': 28.0,

    # Mobility
    'wheel_d': 64.0, 'wheel_t': 18.0,
    'wheel_x_active': 98.0, 'wheel_y': 18.0, 'wheel_z_active': 32.0,
    'wheel_x_stowed': 80.0, 'wheel_z_stowed': 20.0,
    'wheel_fold_deg': 55.0,
    'caster_ball_d': 25.4, 'caster_y': -53.0,

    # Arms
    'shoulder_x': 91.0, 'shoulder_y': -2.0, 'shoulder_z': 154.0,
    'upper_arm_len': 72.0, 'forearm_outer_len': 82.0, 'telescope_stroke': 55.0,
    'upper_arm_d': 31.0, 'forearm_outer_d': 29.0, 'forearm_inner_d': 22.0,
    'wrist_interface_d': 30.0, 'wrist_collar_d': 35.0,

    # Selected component envelopes
    'sts3215': [45.2,24.7,35.0],
    'hl2915': [34.0,20.0,23.0],
    'sc09': [23.2,12.0,25.5],
    'n20_env': [18.0,16.0,39.0],
    'wheel_motor': [25.0,25.0,54.0],
    'speaker': [40.0,28.4,13.0],
    'esp32': [18.0,25.5,3.1],
    'stm32_board_env': [74.0,48.0,13.0],
    'main_board_env': [88.0,56.0,14.0],
    'power_board_env': [92.0,54.0,16.0],
    'vl53_env': [8.0,8.0,5.0], # includes carrier/optical window reservation, not bare IC

    # 3S2P M35A physical pack: 3 cells across, 2 high, cells longitudinal Y
    'cell_d': 18.6, 'cell_l': 65.2,
    'battery_env': [62.0,72.0,43.0],

    # Service / cable architecture
    'rear_service_w': 118.0, 'rear_service_h': 78.0,
    'battery_door_w': 80.0, 'battery_door_h': 48.0,
    'pan_hollow_d': 20.0,
    'cable_channel_w': 10.0, 'cable_channel_d': 8.0,

    # Hidden belly RGB information/light matrix (secret-until-lit)
    # Prototype carrier: Adafruit 13x9 IS31FL3741 51.3 x 39.0 x 4.6 mm.
    # CAD reserves a larger window/keepout so final custom matrix may become wider without recutting the torso.
    'belly_window_w': 74.0, 'belly_window_h': 46.0, 'belly_window_t': 1.2,
    'belly_matrix_keepout_w': 80.0, 'belly_matrix_keepout_h': 54.0, 'belly_matrix_keepout_d': 12.0,
    'belly_matrix_proto_w': 51.3, 'belly_matrix_proto_h': 39.0, 'belly_matrix_proto_t': 4.6,
    'belly_matrix_z': 121.0,
}

# ---------- low-level geometry ----------
def vec(p): return cq.Vector(float(p[0]),float(p[1]),float(p[2]))

def rounded_box(w,d,h,r=2.0, center=(0,0,0)):
    r=max(0.1,min(r,w/4,d/4,h/4))
    sh = cq.Workplane('XY').box(w,d,h, centered=(True,True,True)).edges().fillet(r).val()
    return sh.translate(vec(center))

def tapered_box(w0,d0,w1,d1,h,r=4.0,z0=0.0):
    sh=(cq.Workplane('XY').workplane(offset=z0).rect(w0,d0)
        .workplane(offset=h).rect(w1,d1).loft(combine=True))
    if r>0:
        try: sh=sh.edges().fillet(r)
        except Exception: pass
    return sh.val()

def tapered_shell(w0,d0,w1,d1,h,t,r=4.0,z0=0.0, open_top=False, open_bottom=False):
    outer=tapered_box(w0,d0,w1,d1,h,r,z0)
    iz0=z0+(0 if open_bottom else t)
    ih=h-(0 if open_top else t)-(0 if open_bottom else t)
    # keep walls roughly constant; inner section dims reduced by 2t
    iw0=max(2,w0-2*t); id0=max(2,d0-2*t); iw1=max(2,w1-2*t); id1=max(2,d1-2*t)
    inner=tapered_box(iw0,id0,iw1,id1,ih,max(0.5,r-t),iz0)
    return outer.cut(inner)

def cylinder_between(p1,p2,r):
    p1v=vec(p1); p2v=vec(p2); dv=p2v-p1v; L=dv.Length
    if L < 1e-6: return cq.Solid.makeSphere(r,p1v)
    sh=cq.Solid.makeCylinder(r,L,p1v,dv.normalized())
    return sh.fuse(cq.Solid.makeSphere(r,p1v)).fuse(cq.Solid.makeSphere(r,p2v))

def box_between(p1,p2,width,depth=None, fillet=2):
    # capsule-like link used for robust arm packaging
    return cylinder_between(p1,p2,width/2)

def tube_z(od,id,h,center=(0,0,0)):
    out=cq.Solid.makeCylinder(od/2,h,vec((center[0],center[1],center[2]-h/2)),vec((0,0,1)))
    inn=cq.Solid.makeCylinder(id/2,h+2,vec((center[0],center[1],center[2]-h/2-1)),vec((0,0,1)))
    return out.cut(inn)

def ring_axis_x(od,id,length,center=(0,0,0)):
    p=(center[0]-length/2,center[1],center[2])
    out=cq.Solid.makeCylinder(od/2,length,vec(p),vec((1,0,0)))
    inn=cq.Solid.makeCylinder(id/2,length+2,vec((p[0]-1,p[1],p[2])),vec((1,0,0)))
    return out.cut(inn)

def motor_cylinder_x(length=54,d=25,center=(0,0,0)):
    return cq.Solid.makeCylinder(d/2,length,vec((center[0]-length/2,center[1],center[2])),vec((1,0,0)))

def board(w,h,t,center):
    # boards lie in X-Z plane (thin in Y) when used in rear service bay
    return rounded_box(w,t,h,1.2,center)

def color(hexstr):
    h=hexstr.lstrip('#'); return cq.Color(int(h[0:2],16)/255,int(h[2:4],16)/255,int(h[4:6],16)/255)

C={
 'shell':color('#E7E4DE'), 'dark':color('#252A30'),'black':color('#0B1015'),'metal':color('#70777D'),
 'structure':color('#4B5157'),'battery':color('#348B4E'),'pcb':color('#2D6C9F'),'power':color('#D88730'),
 'sensor':color('#21B6C7'),'motor':color('#AA4A2C'),'servo':color('#C35D31'),'tire':color('#17191D'),
 'tool':color('#424950'),'accent':color('#55BDEB'),'light':color('#7EDCFF'),'keepout':cq.Color(0.4,0.7,1.0,0.3),'cable':color('#D7A920')
}

# ---------- core printable / structural parts ----------
def lower_chassis():
    plate=rounded_box(154,108,5,6,(0,3,13))
    # structural ribs, battery rail supports, wheel hinge blocks
    ribs=rounded_box(142,8,18,2,(0,2,23))
    cross=rounded_box(8,94,18,2,(0,2,23))
    return plate.fuse(ribs).fuse(cross)

def shutdown_feet():
    feet=[]
    for x in (-62,62):
        for y in (-48,48):
            feet.append(rounded_box(22,16,7,3,(x,y,4.5)))
    return feet

def base_shell():
    t=P['wall_cosmetic']; z0=0
    shell=tapered_shell(P['base_w'],P['base_d'],P['base_w']-8,P['base_d']-6,P['base_h'],t,14,z0,open_top=True)
    # side wheel pod openings
    for sx in (-1,1):
        cut=rounded_box(36,76,48,10,(sx*(P['base_w']/2-5),P['wheel_y'],31))
        shell=shell.cut(cut)
    # underside caster opening + speaker vent region
    caster_cut=cq.Solid.makeCylinder(16,16,vec((0,P['caster_y'],0)),vec((0,0,1)))
    shell=shell.cut(caster_cut)
    # hidden lower-front acoustic slots
    for x in (-18,-9,0,9,18):
        slot=rounded_box(3,18,4,1,(x,-P['base_d']/2+4,11))
        shell=shell.cut(slot)
    return shell

def torso_shell():
    t=P['wall_cosmetic']; z0=P['body_z0']
    shell=tapered_shell(P['torso_bottom_w'],P['torso_bottom_d'],P['torso_top_w'],P['torso_top_d'],P['torso_h'],t,16,z0,open_bottom=True)
    top=P['body_z0']+P['torso_h']
    # head bay opening and swept clearance downward
    head_cut=rounded_box(P['head_bay_w'],P['head_bay_d'],96,10,(0,-1,top-42))
    shell=shell.cut(head_cut)
    # side shoulder / arm stow recesses
    for sx in (-1,1):
        cut=rounded_box(34,70,116,11,(sx*(P['torso_bottom_w']/2-5),-4,110))
        shell=shell.cut(cut)
    # hidden belly light-matrix optical insert opening on front (-Y). The opening is structural only;
    # the external visible surface is a flush secret-until-lit PC/PMMA insert, not a visible screen bezel.
    belly_cut=rounded_box(P['belly_window_w']+1.0,10,P['belly_window_h']+1.0,8,(0,-P['torso_bottom_d']/2+1,P['belly_matrix_z']))
    shell=shell.cut(belly_cut)
    # rear service opening
    service=rounded_box(P['rear_service_w'],10,P['rear_service_h'],8,(0,P['torso_bottom_d']/2-1,122))
    shell=shell.cut(service)
    # lower rear battery access opening
    bdoor=rounded_box(P['battery_door_w'],10,P['battery_door_h'],7,(0,P['torso_bottom_d']/2-2,69))
    shell=shell.cut(bdoor)
    # lower hidden ventilation slots along rear-side
    for x in (-50,-35,35,50):
        slot=rounded_box(3,10,22,1,(x,P['torso_bottom_d']/2-2,94))
        shell=shell.cut(slot)
    return shell

def rear_service_cover():
    # cosmetic/service cover, no wiring attached
    return rounded_box(P['rear_service_w']-1.0,3.0,P['rear_service_h']-1.0,7,(0,P['torso_bottom_d']/2+0.5,122))

def battery_door():
    return rounded_box(P['battery_door_w']-1.0,3.0,P['battery_door_h']-1.0,6,(0,P['torso_bottom_d']/2+0.5,69))

def shoulder_tower(side):
    sx=side*65
    tower=rounded_box(23,34,103,5,(sx,16,120))
    topbridge=rounded_box(28,34,18,4,(sx,8,169))
    # M3 access holes / adapter grid simplified
    return tower.fuse(topbridge)

def rear_bridge():
    return rounded_box(138,20,18,4,(0,22,170))

def head_lift_rail():
    # MGN7-class reservation: rail + two carriage blocks
    rail=rounded_box(7,7,112,1,(0,24,124))
    return rail

def head_leadscrew():
    return cq.Solid.makeCylinder(2.5,112,vec((14,24,68)),vec((0,0,1)))

def head_lift_motor():
    # N20 envelope at bottom/rear of head bay
    return rounded_box(*P['n20_env'],3,(30,24,92))

def head_carriage(zring):
    # moving plate around lift rail, supports pan assembly
    plate=rounded_box(78,37,7,5,(0,5,zring-2))
    guide=rounded_box(18,20,28,3,(0,22,zring-20))
    return plate.fuse(guide)

def pan_ring(zring):
    return tube_z(54,P['pan_hollow_d'],10,(0,0,zring))

def pan_servo(zring):
    return rounded_box(*P['sc09'],2,(-35,7,zring-2))

def head_shell_shape(head_bottom, screen_open=True):
    t=P['wall_cosmetic']; h=P['head_h']; z0=head_bottom
    outer=tapered_box(P['head_w']-4,P['head_d']-2,P['head_w'],P['head_d'],h,9,z0)
    inner=tapered_box(P['head_w']-4-2*t,P['head_d']-2-2*t,P['head_w']-2*t,P['head_d']-2*t,h-2*t,6,z0+t)
    shell=outer.cut(inner)
    if screen_open:
        # display opening on front (-Y), landscape, plus camera brow opening
        screen_cut=rounded_box(P['display_w']+2.0,12,P['display_h']+2.0,4,(0,-P['head_d']/2+1,z0+34))
        shell=shell.cut(screen_cut)
        brow=rounded_box(88,12,10,3,(0,-P['head_d']/2+1,z0+72))
        shell=shell.cut(brow)
    # rear head service hatch
    hatch=rounded_box(64,12,38,6,(0,P['head_d']/2-1,z0+38))
    shell=shell.cut(hatch)
    return shell

def head_bezel(head_bottom):
    z=head_bottom+34
    outer=rounded_box(P['display_w']+10,3,P['display_h']+10,6,(0,-P['head_d']/2-1.5,z))
    inner=rounded_box(P['display_w']+1.0,6,P['display_h']+1.0,4,(0,-P['head_d']/2-1.5,z))
    return outer.cut(inner)

def head_rear_hatch(head_bottom):
    return rounded_box(63,3,37,5,(0,P['head_d']/2+0.5,head_bottom+38))

def screen_envelope(head_bottom):
    return rounded_box(P['display_w'],P['display_t'],P['display_h'],1,(0,-P['head_d']/2+5,head_bottom+34))

def camera_envelope(head_bottom):
    w,d,h=P['camera_env']; return rounded_box(w,d,h,2,(0,-2,head_bottom+70))

def lidar_envelope(head_bottom):
    # Prototype carrier envelope. Final custom brow PCB is expected to shrink toward the bare VL53L1X package.
    w,h,d=P['lidar_proto_env']
    return rounded_box(w,d,h,1.5,(P['lidar_x'],-P['head_d']/2+7,head_bottom+70))

def mic_envelopes(head_bottom):
    # actual mic body tiny; envelope includes gasket/port PCB; keep symmetric for beamforming.
    return [rounded_box(8,6,6,1,(-46,-P['head_d']/2+5,head_bottom+71)),
            rounded_box(8,6,6,1,(46,-P['head_d']/2+5,head_bottom+71))]

def tilt_yoke(head_bottom):
    z=head_bottom+P['head_h']/2
    left=ring_axis_x(38,28,5,(-P['head_w']/2+8,0,z))
    right=ring_axis_x(38,28,5,(P['head_w']/2-8,0,z))
    bridge=rounded_box(P['head_w']-20,16,10,3,(0,6,z-31))
    return left.fuse(right).fuse(bridge)

def tilt_servo(head_bottom):
    return rounded_box(*P['sc09'],2,(P['head_w']/2-23,8,head_bottom+P['head_h']/2))

def shutdown_head_seats():
    # three structural seats; pads modeled separately in docs, no latch
    return [rounded_box(18,16,7,3,(-38,-11,106)), rounded_box(18,16,7,3,(38,-11,106)), rounded_box(24,14,7,3,(0,17,106))]

def battery_cassette():
    w,d,h=P['battery_env']; t=2.2
    outer=rounded_box(w,d,h,5,(0,17,34))
    inner=rounded_box(w-2*t,d-2*t,h-2*t,4,(0,17,34+t/2))
    case=outer.cut(inner)
    # rear pull/latch feature
    handle=rounded_box(34,8,10,3,(0,17+d/2+2,34))
    return case.fuse(handle)

def battery_cells():
    # 3 across X, 2 high Z, axes along Y
    cells=[]; d=P['cell_d']; L=P['cell_l'];
    for ix in (-1,0,1):
        for iz in (-0.5,0.5):
            x=ix*(d+0.9); z=34+iz*(d+0.9)
            p=(x,17-L/2,z)
            cells.append(cq.Solid.makeCylinder(d/2,L,vec(p),vec((0,1,0))))
    return cells

def battery_rails():
    rails=[]
    for x in (-33,33):
        rails.append(rounded_box(5,82,6,1,(x,20,14)))
    stop=rounded_box(72,5,10,2,(0,-20,18))
    return rails+[stop]

def electronics_trays():
    # Board envelopes on rear vertical service sled, not vendor-specific final PCBs
    main=board(*P['main_board_env'][:2],P['main_board_env'][2],(0,48,139))
    safety=board(*P['stm32_board_env'][:2],P['stm32_board_env'][2],(0,46,105))
    power=board(*P['power_board_env'][:2],P['power_board_env'][2],(0,43,76))
    return main,safety,power

def esp32_antenna_keepout():
    # upper/rear nonmetallic keepout volume around antenna area
    return rounded_box(34,18,24,2,(36,49,158))

def speaker_enclosure():
    # low-front chamber; speaker face points down-forward
    box=rounded_box(49,37,23,5,(-39,-42,73))
    cavity=rounded_box(44,32,19,4,(-39,-42,74))
    return box.cut(cavity)

def speaker_envelope():
    w,d,h=P['speaker']; return rounded_box(w,d,h,2,(-39,-46,75))

def belly_deadfront_window():
    # Reference geometry for a flush 0.8-1.5 mm optical-grade PC/PMMA insert.
    # Final finish/transmission is selected by optical coupon testing; do not print opaque.
    y=-P['torso_bottom_d']/2-0.1
    return rounded_box(P['belly_window_w'],P['belly_window_t'],P['belly_window_h'],6,(0,y,P['belly_matrix_z']))

def belly_matrix_proto_envelope():
    # Adafruit 13x9 carrier used only as a development envelope; custom PCB can use the larger reserved keepout.
    y=-P['torso_bottom_d']/2+8.0
    return rounded_box(P['belly_matrix_proto_w'],P['belly_matrix_proto_t'],P['belly_matrix_proto_h'],2,(0,y,P['belly_matrix_z']))

def belly_matrix_keepout():
    y=-P['torso_bottom_d']/2+10.0
    return rounded_box(P['belly_matrix_keepout_w'],P['belly_matrix_keepout_d'],P['belly_matrix_keepout_h'],5,(0,y,P['belly_matrix_z']))

def belly_pixel_baffle():
    # Black internal light well; protects readability and suppresses glow into torso seams/electronics.
    # Prototype is a shallow printable frame, not 117 individual cells. Final custom PCB may add pixel-cell ribs.
    y=-P['torso_bottom_d']/2+5.2
    outer=rounded_box(P['belly_window_w']-2,8,P['belly_window_h']-2,5,(0,y,P['belly_matrix_z']))
    inner=rounded_box(P['belly_window_w']-7,10,P['belly_window_h']-7,4,(0,y,P['belly_matrix_z']))
    return outer.cut(inner)

def sensor_mounts():
    down=[]; horiz=[]
    for x in (-72,72):
        for y in (-50,50):
            down.append(rounded_box(12,12,6,2,(x,y,8)))
    # four horizontal at front/rear corners
    horiz += [rounded_box(12,6,12,2,(-66,-67,31)), rounded_box(12,6,12,2,(66,-67,31)),
              rounded_box(12,6,12,2,(-66,67,31)), rounded_box(12,6,12,2,(66,67,31))]
    return down,horiz

def bumper_ring():
    # four compliant segments with gaps at wheel pods
    front=rounded_box(116,5,10,2,(0,-P['base_d']/2-2,20))
    rear=rounded_box(116,5,10,2,(0,P['base_d']/2+2,20))
    left=rounded_box(5,72,10,2,(-P['base_w']/2-2,0,20))
    right=rounded_box(5,72,10,2,(P['base_w']/2+2,0,20))
    return [front,rear,left,right]

def ball_caster():
    ball=cq.Solid.makeSphere(P['caster_ball_d']/2,vec((0,P['caster_y'],P['caster_ball_d']/2)))
    cup=tube_z(P['caster_ball_d']+8,P['caster_ball_d']+1,10,(0,P['caster_y'],16))
    return ball,cup

def anti_tip_skids():
    return [rounded_box(42,12,5,2,(0,-P['base_d']/2+5,4.5)), rounded_box(42,12,5,2,(0,P['base_d']/2-5,4.5))]

def wheel_module(side, state='active'):
    sx=1 if side>0 else -1
    x=sx*P['wheel_x_active']; z=P['wheel_z_active']; fold=0 if state=='active' else sx*P['wheel_fold_deg']
    # wheel axis X
    tire=cq.Solid.makeCylinder(P['wheel_d']/2,P['wheel_t'],vec((x-P['wheel_t']/2,y:=P['wheel_y'],z)),vec((1,0,0)))
    hub=cq.Solid.makeCylinder(20,P['wheel_t']+2,vec((x-(P['wheel_t']+2)/2,y,z)),vec((1,0,0)))
    # independent axle bearing sleeve
    axle=ring_axis_x(24,8,22,(x-12*sx,y,z))
    # motor lies parallel to wheel axle, inside pod and above/forward
    mx=x-38*sx; my=y-18; mz=z+17
    motor=motor_cylinder_x(54,25,(mx,my,mz))
    # pulley pair + belt cover approximate
    p1=cq.Solid.makeCylinder(9,7,vec((x-16*sx-3.5,y,z)),vec((1,0,0)))
    p2=cq.Solid.makeCylinder(7,7,vec((mx+13*sx-3.5,my,mz)),vec((1,0,0)))
    cover=rounded_box(12,55,52,7,(x-25*sx,y-8,z+8))
    # trim to pod-sized fairing
    fairing=rounded_box(38,70,72,14,(x-18*sx,y,z+5))
    hinge_x=sx*(P['base_w']/2-10)
    hinge_z=62
    hinge=cq.Solid.makeCylinder(9,26,vec((hinge_x,y-13,hinge_z)),vec((0,1,0))).cut(cq.Solid.makeCylinder(3,28,vec((hinge_x,y-14,hinge_z)),vec((0,1,0))))
    stop=rounded_box(10,20,12,2,(sx*(P['base_w']/2-8),y-18,48))
    lock=rounded_box(8,12,8,2,(sx*(P['base_w']/2-9),y+20,45))
    shapes={'tire':tire,'hub':hub,'axle':axle,'motor':motor,'pulley1':p1,'pulley2':p2,'cover':cover,'fairing':fairing,'hinge':hinge,'stop':stop,'lock':lock}
    if state!='active':
        # rotate pod contents around approximate Y hinge axis for visual stow, then translate toward stored centers
        axis0=(hinge_x,y-1,hinge_z); axis1=(hinge_x,y+1,hinge_z)
        for k in list(shapes):
            if k not in ('hinge','stop','lock'):
                try: shapes[k]=shapes[k].rotate(vec(axis0),vec(axis1),fold)
                except Exception: pass
    return shapes

# ---------- arm mechanisms ----------
def shoulder_mechanism(side):
    sx=side*P['shoulder_x']; sy=P['shoulder_y']; sz=P['shoulder_z']
    # structural bearing pod + two orthogonal STS3215 envelopes
    pod=ring_axis_x(54,28,18,(sx,sy,sz))
    s1=rounded_box(*P['sts3215'],4,(side*76,10,sz))
    # second servo rotated conceptually; envelope orientation swapped X/Y
    s2=rounded_box(P['sts3215'][1],P['sts3215'][0],P['sts3215'][2],4,(side*88,-11,sz-20))
    return pod,s1,s2

def arm_points(side, pose='active', extension=0.0):
    S=(side*P['shoulder_x'],P['shoulder_y'],P['shoulder_z'])
    if pose=='active':
        E=(side*(P['shoulder_x']+13),-34,112)
        R=(side*(P['shoulder_x']+6),-75,104)
    elif pose=='reach':
        E=(side*(P['shoulder_x']+8),-52,116)
        R=(side*(P['shoulder_x']+2),-122,79)
    elif pose=='desk':
        E=(side*(P['shoulder_x']+10),-45,112)
        R=(side*(P['shoulder_x']+12),-88,68)
    else: # stow, compact in body side recess
        E=(side*(P['shoulder_x']-1),1,102)
        R=(side*(P['shoulder_x']-13),-2,128)
    # extension follows E->R direction outward from R
    ex=max(0,min(P['telescope_stroke'],extension))
    dv=cq.Vector(R[0]-E[0],R[1]-E[1],R[2]-E[2]); n=dv.normalized()
    T=(R[0]+n.x*ex,R[1]+n.y*ex,R[2]+n.z*ex)
    return S,E,R,T

def arm_mechanism(side,pose='active',extension=0.0,tool='gripper'):
    S,E,R,T=arm_points(side,pose,extension)
    # Shoulder pod and upper arm cosmetic/structural housing
    pod,s1,s2=shoulder_mechanism(side)
    upper=box_between(S,E,P['upper_arm_d'])
    elbow=cq.Solid.makeSphere(20,vec(E))
    fore=box_between(E,R,P['forearm_outer_d'])
    inner=box_between(R,T,P['forearm_inner_d']) if extension>0.5 else cq.Solid.makeSphere(P['forearm_inner_d']/2,vec(R))
    # proximal elbow motor (HL2915) near shoulder, plus belt cover
    mpos=((S[0]*0.7+E[0]*0.3),(S[1]*0.7+E[1]*0.3),(S[2]*0.7+E[2]*0.3))
    elbow_motor=rounded_box(*P['hl2915'],3,mpos)
    belt=box_between(mpos,E,11)
    # roll module at forearm distal end; wrist yoke
    roll=cq.Solid.makeCylinder(17,24,vec((T[0]-12,T[1],T[2])),vec((1,0,0))) if side>0 else cq.Solid.makeCylinder(17,24,vec((T[0]-12,T[1],T[2])),vec((1,0,0)))
    wrist=cq.Solid.makeSphere(17,vec(T))
    # quick-swap axis approximated downward/front from T
    q=(T[0],T[1]-3,T[2]-18)
    quick=tube_z(P['wrist_collar_d'],P['wrist_interface_d']-4,14,q)
    # SC09 roll/wrist envelopes adjacent to T
    roll_servo=rounded_box(*P['sc09'],2,(T[0]-side*14,T[1]+12,T[2]+5))
    wrist_servo=rounded_box(*P['sc09'],2,(T[0],T[1]+10,T[2]-18))
    shapes={'pod':pod,'shoulder_servo_1':s1,'shoulder_servo_2':s2,'upper':upper,'elbow':elbow,'forearm':fore,'inner':inner,
            'elbow_motor':elbow_motor,'elbow_belt':belt,'roll':roll,'wrist':wrist,'quick':quick,'roll_servo':roll_servo,'wrist_servo':wrist_servo}
    # tools
    if tool=='gripper': shapes.update(gripper_tool(q))
    elif tool=='magnet': shapes.update(magnet_tool(q))
    return shapes

def quickswap_female():
    # Printable wrist-side collar prototype. 3-lug bayonet + 8 contact bores.
    outer=tube_z(35,24,16,(0,0,0))
    # three bayonet entry windows
    sh=outer
    for a in (0,120,240):
        rad=math.radians(a); x=14*math.cos(rad); y=14*math.sin(rad)
        sh=sh.cut(rounded_box(8,7,6,1,(x,y,3)))
    # 8 pogo bores in circle
    for i in range(8):
        a=2*math.pi*i/8; x=10*math.cos(a); y=10*math.sin(a)
        bore=cq.Solid.makeCylinder(1.25,20,vec((x,y,-10)),vec((0,0,1)))
        sh=sh.cut(bore)
    return sh

def quickswap_male():
    core=tube_z(29,18,12,(0,0,0))
    # three structural lugs
    sh=core
    for a in (0,120,240):
        rad=math.radians(a); x=14*math.cos(rad); y=14*math.sin(rad)
        lug=rounded_box(7,5,5,1,(x,y,2)).rotate(vec((0,0,0)),vec((0,0,1)),a)
        sh=sh.fuse(lug)
    # asymmetric key
    sh=sh.fuse(rounded_box(4,3,7,1,(0,-15,0)))
    return sh

def gripper_tool(q=(0,0,0)):
    x,y,z=q
    palm=rounded_box(34,28,28,5,(x,y,z-22))
    servo=rounded_box(*P['sc09'],2,(x,y+7,z-22))
    # two 2-phalange fingers, spring-open neutral slight inward curve
    shapes={'gripper_palm':palm,'gripper_servo':servo}
    for side in (-1,1):
        p0=(x+side*11,y-2,z-30); p1=(x+side*14,y-5,z-52); p2=(x+side*10,y-8,z-70)
        shapes[f'finger_{side}_prox']=cylinder_between(p0,p1,5)
        shapes[f'finger_{side}_dist']=cylinder_between(p1,p2,4.5)
    return shapes

def magnet_tool(q=(0,0,0)):
    x,y,z=q
    housing=rounded_box(30,30,30,6,(x,y,z-23))
    magnet=cq.Solid.makeCylinder(10,15,vec((x,y,z-45)),vec((0,0,1)))
    bumper=tube_z(26,20.8,3,(x,y,z-45))
    return {'magnet_housing':housing,'electromagnet':magnet,'magnet_bumper':bumper}

def head_harness_keepout(state='active'):
    hb={'active':P['head_active_bottom'],'sleep':P['head_sleep_bottom'],'shutdown':P['head_shutdown_bottom']}[state]
    # controlled vertical loop at rear, not raw cable geometry
    return rounded_box(16,12,76,4,(0,17,(hb+P['body_z0']+P['torso_h'])/2))

def arm_cable_keepout(side,pose='active'):
    S,E,R,T=arm_points(side,pose,20 if pose!='stow' else 0)
    return [cylinder_between(S,E,5),cylinder_between(E,R,5),cylinder_between(R,T,4)]


# ---------- functional prototype parts (manufacturing-oriented) ----------
def wheel_pod_printable():
    # local origin: wheel axle along X, Y front/rear, Z up. Two 608-class bearing seats.
    outer=rounded_box(40,70,72,10,(0,0,0))
    inner=rounded_box(32,60,62,8,(0,0,2))
    sh=outer.cut(inner)
    # 8 mm axle through-bore + 22.2 mm bearing pockets from both sides
    sh=sh.cut(cq.Solid.makeCylinder(4.2,44,vec((-22,0,-5)),vec((1,0,0))))
    sh=sh.cut(cq.Solid.makeCylinder(11.1,7.5,vec((-20.5,0,-5)),vec((1,0,0))))
    sh=sh.cut(cq.Solid.makeCylinder(11.1,7.5,vec((13,0,-5)),vec((1,0,0))))
    # 25 mm motor pass-through cradle and strap holes, parallel to axle and offset up/front
    sh=sh.cut(cq.Solid.makeCylinder(12.8,44,vec((-22,-18,15)),vec((1,0,0))))
    # upper-inner longitudinal hinge bore along Y
    sh=sh.cut(cq.Solid.makeCylinder(3.2,76,vec((-13,-38,29)),vec((0,1,0))))
    # lock pin bore and hard-stop relief
    sh=sh.cut(cq.Solid.makeCylinder(2.2,14,vec((-18,20,18)),vec((1,0,0))))
    return sh

def shoulder_bearing_yoke_printable():
    # structural output yoke; servo torque enters by belt, bearings carry arm load
    outer=ring_axis_x(54,26,22,(0,0,0))
    # 608-class seats each side; center remains clear for axle/cable routing
    sh=outer.cut(cq.Solid.makeCylinder(11.1,7.2,vec((-11.1,0,0)),vec((1,0,0))))
    sh=sh.cut(cq.Solid.makeCylinder(11.1,7.2,vec((3.9,0,0)),vec((1,0,0))))
    # belt pulley attachment flange
    sh=sh.fuse(tube_z(36,28,7,(0,0,-16)).rotate(vec((0,0,0)),vec((0,1,0)),90))
    return sh

def upper_arm_housing_printable():
    L=P['upper_arm_len']; t=2.6
    outer=rounded_box(38,34,L,8,(0,0,L/2))
    inner=rounded_box(38-2*t,34-2*t,L-7,6,(0,0,L/2+2))
    sh=outer.cut(inner)
    # HL2915 service/mount pocket near proximal shoulder side
    servo_cut=rounded_box(35,22,25,4,(0,5,L-18))
    sh=sh.cut(servo_cut)
    # enclosed toothed-belt corridor toward elbow
    sh=sh.cut(rounded_box(12,14,L-20,3,(0,-7,L/2-4)))
    # shoulder/elbow axle bores
    for z in (L-6,6):
        sh=sh.cut(cq.Solid.makeCylinder(4.1,40,vec((-20,0,z)),vec((1,0,0))))
    # removable cover screw pilots
    for x in (-13,13):
        for z in (20,L-20):
            sh=sh.cut(cq.Solid.makeCylinder(1.7,6,vec((x,-18,z)),vec((0,1,0))))
    return sh

def forearm_outer_printable():
    L=P['forearm_outer_len']; t=2.6
    outer=rounded_box(34,30,L,6,(0,0,L/2))
    inner=rounded_box(27.2,23.2,L+2,4,(0,0,L/2))
    sh=outer.cut(inner)
    # wear-pad access pockets at two spaced guide stations
    for z in (15,L-15):
        for x in (-15.5,15.5): sh=sh.cut(rounded_box(3.5,13,10,1,(x,0,z)))
        for y in (-13.5,13.5): sh=sh.cut(rounded_box(13,3.5,10,1,(0,y,z)))
    # lead screw / cable service opening at proximal end
    sh=sh.cut(cq.Solid.makeCylinder(4.5,12,vec((0,0,-2)),vec((0,0,1))))
    # wiper/dust lip seat at distal opening
    lip=rounded_box(36,32,3,5,(0,0,L-1.5)).cut(rounded_box(27.6,23.6,5,4,(0,0,L-1.5)))
    return sh.fuse(lip)

def forearm_inner_printable():
    # inner slider remains long enough for >= ~40 mm structural overlap at full 55 mm stroke
    L=96.0
    sh=rounded_box(25.4,21.4,L,4,(0,0,L/2))
    # separate cable corridor, away from lead-screw nut
    sh=sh.cut(rounded_box(6.5,8,L-8,2,(7,0,L/2+2)))
    # T5 nut pocket near proximal end and lead-screw pass bore
    sh=sh.cut(rounded_box(12,11,10,2,(0,0,10)))
    sh=sh.cut(cq.Solid.makeCylinder(3.1,20,vec((0,0,0)),vec((0,0,1))))
    # hard-stop shoulder / distal wrist flange
    sh=sh.fuse(rounded_box(29,25,5,4,(0,0,L-2.5)))
    # roll bearing/hollow shaft reservation at distal end
    sh=sh.cut(cq.Solid.makeCylinder(8.1,30,vec((-15,0,L-10)),vec((1,0,0))))
    return sh

def wrist_pitch_yoke_printable():
    # U-yoke with dual 688-class bearing seats (8x16x5 nominal)
    outer=rounded_box(38,28,34,7,(0,0,0))
    center=rounded_box(24,32,24,5,(0,0,1))
    sh=outer.cut(center)
    sh=sh.cut(cq.Solid.makeCylinder(8.1,40,vec((-20,0,0)),vec((1,0,0))))
    # two local bearing counterbores
    sh=sh.cut(cq.Solid.makeCylinder(8.1,5.5,vec((-19,0,0)),vec((1,0,0))))
    sh=sh.cut(cq.Solid.makeCylinder(8.1,5.5,vec((13.5,0,0)),vec((1,0,0))))
    return sh

def head_carriage_printable():
    z=0
    plate=rounded_box(78,37,7,5,(0,0,0))
    guide=rounded_box(22,20,31,3,(0,17,-18))
    sh=plate.fuse(guide)
    # MGN7 carriage adapter screw grid, deliberately slotted for shimming/alignment
    for x in (-6,6):
        for zz in (-25,-13):
            sh=sh.cut(rounded_box(3.2,8,5.5,1,(x,17,zz)))
    # T5 nut mount / service bore
    sh=sh.cut(cq.Solid.makeCylinder(5.5,10,vec((14,-5,-5)),vec((0,0,1))))
    # hollow pan cable aperture
    sh=sh.cut(cq.Solid.makeCylinder(P['pan_hollow_d']/2+2,12,vec((0,0,-6)),vec((0,0,1))))
    return sh

def quickswap_safety_collar():
    sh=tube_z(39,34.4,10,(0,0,0))
    # pull tabs opposite each other
    sh=sh.fuse(rounded_box(8,5,8,2,(21,0,0))).fuse(rounded_box(8,5,8,2,(-21,0,0)))
    # three anti-rotation/spring guide notches
    for a in (0,120,240):
        rad=math.radians(a); x=18*math.cos(rad); y=18*math.sin(rad)
        sh=sh.cut(rounded_box(3,5,6,1,(x,y,0)).rotate(vec((0,0,0)),vec((0,0,1)),a))
    return sh

def gripper_palm_printable():
    outer=rounded_box(36,30,31,5,(0,0,0)); inner=rounded_box(31,25,26,4,(0,1,1))
    sh=outer.cut(inner)
    # SC09 pocket + wire exit
    sh=sh.cut(rounded_box(24,13,26,2,(0,5,0)))
    sh=sh.cut(rounded_box(8,8,8,2,(0,15,-5)))
    # finger pivots along Y
    for x in (-11,11): sh=sh.cut(cq.Solid.makeCylinder(1.6,34,vec((x,-17,-10)),vec((0,1,0))))
    # capstan/tendon guide bore
    sh=sh.cut(cq.Solid.makeCylinder(3.2,34,vec((0,-17,4)),vec((0,1,0))))
    return sh

def gripper_finger_prox_printable():
    sh=rounded_box(11,9,31,4,(0,0,-15))
    # base and distal pivot bores along Y
    for z in (-4,-28): sh=sh.cut(cq.Solid.makeCylinder(1.6,13,vec((0,-6.5,z)),vec((0,1,0))))
    # tendon channel + soft-pad recess
    sh=sh.cut(cq.Solid.makeCylinder(0.9,30,vec((2,0,-30)),vec((0,0,1))))
    sh=sh.cut(rounded_box(7,2,18,1,(0,-4.5,-16)))
    return sh

def gripper_finger_dist_printable():
    sh=rounded_box(10,8,27,4,(0,0,-13.5))
    sh=sh.cut(cq.Solid.makeCylinder(1.6,12,vec((0,-6,-3)),vec((0,1,0))))
    # inward precision V/tip relief and pad recess
    sh=sh.cut(rounded_box(6,2,15,1,(0,-4,-14)))
    return sh

def magnet_tool_housing_printable():
    outer=rounded_box(32,32,32,6,(0,0,0)); inner=rounded_box(27,27,27,5,(0,0,2))
    sh=outer.cut(inner)
    # P20/15 pole-face cavity opening from bottom
    sh=sh.cut(cq.Solid.makeCylinder(10.3,19,vec((0,0,-16)),vec((0,0,1))))
    # 3 spring-float guide bores around magnet
    for a in (0,120,240):
        rad=math.radians(a); x=12*math.cos(rad); y=12*math.sin(rad)
        sh=sh.cut(cq.Solid.makeCylinder(1.6,34,vec((x,y,-17)),vec((0,0,1))))
    # wire/temperature sensor channel
    sh=sh.cut(rounded_box(6,8,8,2,(0,14,7)))
    return sh


# ---------- Assembly creation ----------
def add(assy,shape,name,col): assy.add(shape,name=name,color=col)

def build_assembly(state='ACTIVE', cutaway=False):
    st=state.upper(); assy=cq.Assembly(name=f'ZI-E_{st}')
    # outer shells / structure
    if not cutaway:
        add(assy,base_shell(),'base_shell',C['shell']); add(assy,torso_shell(),'torso_shell',C['shell'])
        add(assy,rear_service_cover(),'rear_service_cover',C['shell']); add(assy,battery_door(),'battery_door',C['shell'])
    else:
        # omit shell for readable cutaway; structure stays
        pass
    add(assy,lower_chassis(),'lower_chassis',C['structure'])
    for fidx,f in enumerate(shutdown_feet()): add(assy,f,f'shutdown_foot_{fidx}',C['dark'])
    add(assy,rear_bridge(),'rear_bridge',C['structure'])
    for side in (-1,1): add(assy,shoulder_tower(side),f'shoulder_tower_{side}',C['structure'])

    # Battery and internals
    add(assy,battery_cassette(),'battery_cassette',C['dark'])
    for i,c in enumerate(battery_cells()): add(assy,c,f'M35A_cell_{i+1}',C['battery'])
    for i,r in enumerate(battery_rails()): add(assy,r,f'battery_rail_{i}',C['structure'])
    main,safety,power=electronics_trays(); add(assy,main,'main_multimedia_board_env',C['pcb']); add(assy,safety,'safety_motion_board_env',C['power']); add(assy,power,'power_distribution_board_env',C['power'])
    add(assy,esp32_antenna_keepout(),'esp32_antenna_keepout',C['keepout'])
    add(assy,speaker_enclosure(),'speaker_chamber',C['dark']); add(assy,speaker_envelope(),'PUI_AS04004PO_env',C['pcb'])
    # Hidden belly RGB information/light matrix. Window stays visually quiet when off; PCB is non-safety multimedia.
    add(assy,belly_deadfront_window(),'belly_deadfront_window',C['shell'])
    add(assy,belly_pixel_baffle(),'belly_matrix_baffle',C['black'])
    add(assy,belly_matrix_proto_envelope(),'IS31FL3741_13x9_proto_env',C['light'])
    add(assy,belly_matrix_keepout(),'belly_matrix_future_keepout',C['keepout'])

    # Sensors/bumper
    down,horiz=sensor_mounts()
    for i,s in enumerate(down): add(assy,s,f'cliff_sensor_{i+1}',C['sensor'])
    for i,s in enumerate(horiz): add(assy,s,f'proximity_sensor_{i+1}',C['sensor'])
    for i,b in enumerate(bumper_ring()): add(assy,b,f'bumper_segment_{i+1}',C['dark'])
    ball,cup=ball_caster(); add(assy,ball,'ball_caster_ball',C['metal']); add(assy,cup,'ball_caster_cartridge',C['dark'])
    for i,s in enumerate(anti_tip_skids()): add(assy,s,f'anti_tip_skid_{i}',C['dark'])

    # Wheels
    wheelstate='stowed' if st=='SHUTDOWN' else 'active'
    for side in (-1,1):
        wm=wheel_module(side,wheelstate)
        for k,sh in wm.items():
            col=C['tire'] if k=='tire' else (C['motor'] if k=='motor' else (C['metal'] if k in ('hub','axle','pulley1','pulley2','hinge') else C['dark']))
            add(assy,sh,f'wheel_{side}_{k}',col)

    # Head lift mechanism
    add(assy,head_lift_rail(),'head_lift_rail',C['metal']); add(assy,head_leadscrew(),'T5x2_head_leadscrew',C['metal']); add(assy,head_lift_motor(),'head_lift_N20',C['motor'])
    hb={'ACTIVE':P['head_active_bottom'],'SLEEP':P['head_sleep_bottom'],'SHUTDOWN':P['head_shutdown_bottom']}[st]
    zring=hb-6
    add(assy,head_carriage(zring),'head_carriage',C['structure']); add(assy,pan_ring(zring),'pan_bearing_ring',C['metal']); add(assy,pan_servo(zring),'pan_SC09',C['servo'])
    add(assy,tilt_yoke(hb),'tilt_yoke',C['structure']); add(assy,tilt_servo(hb),'tilt_SC09',C['servo'])
    add(assy,head_shell_shape(hb),'head_shell',C['shell']); add(assy,head_bezel(hb),'head_bezel',C['dark']); add(assy,head_rear_hatch(hb),'head_rear_hatch',C['shell'])
    add(assy,screen_envelope(hb),'display_3p5_ILI9488_env',C['black']); add(assy,camera_envelope(hb),'OV5640_AF_env',C['pcb']); add(assy,lidar_envelope(hb),'VL53L1X_laser_rangefinder_env',C['sensor'])
    for i,m in enumerate(mic_envelopes(hb)): add(assy,m,f'I2S_mic_{i+1}_env',C['sensor'])
    for i,s in enumerate(shutdown_head_seats()): add(assy,s,f'head_shutdown_seat_{i+1}',C['structure'])
    add(assy,head_harness_keepout(st.lower()),'head_dynamic_harness_keepout',C['keepout'])

    # Arms
    pose='stow' if st=='SHUTDOWN' else ('active' if st=='ACTIVE' else 'stow')
    for side in (-1,1):
        am=arm_mechanism(side,pose,0,'gripper')
        for k,sh in am.items():
            col=C['servo'] if 'servo' in k or 'motor' in k else (C['tool'] if k.startswith('gripper') or k.startswith('finger') or k=='quick' else (C['metal'] if k in ('pod','elbow','roll','wrist') else C['shell']))
            add(assy,sh,f'arm_{side}_{k}',col)
        for i,ck in enumerate(arm_cable_keepout(side,pose)): add(assy,ck,f'arm_{side}_cable_keepout_{i}',C['keepout'])
    return assy

# ---------- printable prototype parts ----------
def printable_parts():
    parts={
      'base_shell':base_shell(), 'torso_shell':torso_shell(), 'rear_service_cover':rear_service_cover(), 'battery_door':battery_door(),
      'lower_chassis':lower_chassis(),'rear_bridge':rear_bridge(),'shoulder_tower_L':shoulder_tower(-1),'shoulder_tower_R':shoulder_tower(1),
      'battery_cassette':battery_cassette(),'speaker_chamber':speaker_enclosure(),
      'belly_matrix_baffle':belly_pixel_baffle(),'belly_deadfront_window_REFERENCE':belly_deadfront_window(),
      'head_shell':head_shell_shape(P['head_active_bottom']),'head_bezel':head_bezel(P['head_active_bottom']),'head_rear_hatch':head_rear_hatch(P['head_active_bottom']),
      'tilt_yoke':tilt_yoke(P['head_active_bottom']),'head_carriage':head_carriage_printable(),
      'quick_swap_female':quickswap_female(),'quick_swap_male':quickswap_male(),'quick_swap_safety_collar':quickswap_safety_collar(),
      'wheel_pod_prototype':wheel_pod_printable(),'shoulder_bearing_yoke':shoulder_bearing_yoke_printable(),
      'upper_arm_housing':upper_arm_housing_printable(),'forearm_outer_keyed':forearm_outer_printable(),'forearm_inner_keyed':forearm_inner_printable(),
      'wrist_pitch_yoke':wrist_pitch_yoke_printable(),'gripper_palm':gripper_palm_printable(),'gripper_finger_prox':gripper_finger_prox_printable(),
      'gripper_finger_dist':gripper_finger_dist_printable(),'magnet_tool_housing_float':magnet_tool_housing_printable(),
    }
    # shutdown feet + bumper segments + skids
    for i,p in enumerate(shutdown_feet()): parts[f'shutdown_foot_{i+1}']=p
    for i,p in enumerate(bumper_ring()): parts[f'bumper_segment_{i+1}']=p
    for i,p in enumerate(anti_tip_skids()): parts[f'anti_tip_skid_{i+1}']=p
    # wheel fairing/cover unique sides active geometry
    for side,label in [(-1,'L'),(1,'R')]:
        wm=wheel_module(side,'active'); parts[f'wheel_fairing_{label}']=wm['fairing']; parts[f'wheel_belt_cover_{label}']=wm['cover']
    # simplified arm shell/link prototypes from active pose; mirror-specific assemblies can share geometry in slicer after reorientation
    for side,label in [(-1,'L'),(1,'R')]:
        am=arm_mechanism(side,'active',0,'gripper')
        for k in ('upper','forearm','inner'):
            parts[f'arm_{label}_{k}_shell']=am[k]
    return parts

# ---------- analytics ----------
def bbox_of_assy(assy):
    b=None
    for ch in assy.objects.values():
        try: bb=ch.obj.BoundingBox()
        except Exception: continue
        if b is None: b=[bb.xmin,bb.ymin,bb.zmin,bb.xmax,bb.ymax,bb.zmax]
        else: b=[min(b[0],bb.xmin),min(b[1],bb.ymin),min(b[2],bb.zmin),max(b[3],bb.xmax),max(b[4],bb.ymax),max(b[5],bb.zmax)]
    return b

def mass_model():
    # Masses are engineering estimates for stability, explicitly tagged by confidence/source.
    # Positions are assembly centroids in mm; measured masses will replace estimates during prototype verification.
    rows=[]
    def r(name,mass,x,y,z,confidence,source): rows.append(dict(item=name,mass_g=mass,x_mm=x,y_mm=y,z_mm=z,confidence=confidence,source=source))
    r('3S2P M35A cells',288,0,17,34,'high','6 x 48 g max Molicel datasheet')
    r('battery cassette+BMS+wiring',65,0,17,35,'medium','CAD/engineering estimate')
    r('left wheel motor',91,-73,-6,49,'medium','25Dx54 class reference mass')
    r('right wheel motor',91,73,-6,49,'medium','25Dx54 class reference mass')
    r('wheels+hubs+pods',100,0,18,32,'low','50 g each estimated')
    r('lower chassis/base shell/feet',310,0,2,35,'low','prototype print estimate')
    r('torso shell+structural towers',330,0,6,116,'low','prototype print estimate')
    r('electronics+wiring',145,0,40,112,'low','PCB/connector estimate')
    r('speaker+chamber',42,-39,-42,74,'medium','speaker vendor mass + chamber estimate')
    r('belly matrix+deadfront+baffle',22,0,-61,P['belly_matrix_z'],'medium','Adafruit board 5.8 g + optical/baffle/harness estimate')
    r('moving head complete',220,0,0,P['head_active_bottom']+41,'low','display/camera/frame/servos estimate')
    r('head laser rangefinder prototype carrier',4,P['lidar_x'],-14,P['head_active_bottom']+70,'medium','VL53L1X carrier allowance; final bare sensor is lighter')
    r('left arm complete',370,-96,-24,100,'low','selected actuator masses + printed links/tool')
    r('right arm complete',370,96,-24,100,'low','selected actuator masses + printed links/tool')
    total=sum(q['mass_g'] for q in rows)
    com=[sum(q['mass_g']*q[k] for q in rows)/total for k in ('x_mm','y_mm','z_mm')]
    return rows,total,com

def write_reports():
    DATA.mkdir(exist_ok=True)
    (DATA/'parameters.json').write_text(json.dumps(P,indent=2),encoding='utf-8')
    # component envelope table
    comp=[
      ['Display candidate','ILI9488 IPS touch','84.52 x 55.26 x 4.0','ElectroPeak listing; touch IC/FPC pinout still verification gate'],
      ['Camera','OV5640 autofocus','26 x 16 x 18 envelope','compact final FPC module target; breakout not packaging authority'],
      ['Head laser rangefinder','VL53L1X ToF laser','25.5 x 17.5 x 4.6 prototype carrier; 4.9 x 2.5 x 1.56 bare sensor','940 nm invisible Class 1; up to 4 m / 50 Hz; dedicated IR window + crosstalk calibration'],
      ['Main MCU','ESP32-S3-WROOM-1-N16R8','18 x 25.5 x 3.1','official module datasheet'],
      ['Safety MCU','STM32G0B1RET6 board envelope','74 x 48 x 13','custom carrier reservation, not final PCB'],
      ['Shoulder servo','Feetech ST-3215-C018','45.2 x 24.7 x 35','official Feetech'],
      ['Elbow servo','Feetech HL-2915-C001','34 x 20 x 23','official Feetech'],
      ['SC09','Waveshare SC09','23.2 x 12.0 x 25.5','official Waveshare'],
      ['Wheel motor','25SG-370CA-78-EN','25D x 54L','RobotShop/E-S Motor'],
      ['Battery cell','Molicel M35A','18.6D max x 65.2H max','official Molicel'],
      ['Speaker','PUI AS04004PO-R','40 x 28.4 x 13','PUI/DigiKey'],
      ['Belly RGB prototype','Adafruit IS31FL3741 13x9','51.3 x 39.0 x 4.6','Adafruit product 5201; final custom matrix reserved by 80 x 54 x 12 keepout'],
      ['Belly optical insert','Secret-until-lit PC/PMMA','74 x 46 x 1.2 nominal','Optical coupon test is authority; 0.8-1.5 mm prototype range'],
    ]
    with open(DATA/'component_envelopes.csv','w',newline='',encoding='utf-8') as f:
        w=csv.writer(f); w.writerow(['Subsystem','Part','CAD envelope mm','Authority/notes']); w.writerows(comp)
    rows,total,com=mass_model()
    with open(DATA/'mass_com.csv','w',newline='',encoding='utf-8') as f:
        w=csv.DictWriter(f,fieldnames=rows[0].keys()); w.writeheader(); w.writerows(rows)
    summary={'estimated_total_mass_g':round(total,1),'estimated_active_COM_mm':{'x':round(com[0],1),'y':round(com[1],1),'z':round(com[2],1)},
             'warning':'Mass/COM is an engineering estimate until components and printed parts are weighed.'}
    (DATA/'mass_com_summary.json').write_text(json.dumps(summary,indent=2),encoding='utf-8')


def export_all():
    write_reports()
    # assemblies in three system states + cutaway
    bboxes={}
    for state in ('ACTIVE','SLEEP','SHUTDOWN'):
        a=build_assembly(state,False); path=CAD/f'ZI-E_v0_3_{state}.step'; a.save(str(path)); bboxes[state]=bbox_of_assy(a)
        # glTF for easy viewing
        try: a.save(str(CAD/f'ZI-E_v0_3_{state}.gltf'))
        except Exception as e: print('gltf warning',state,e)
    cut=build_assembly('ACTIVE',True); cut.save(str(CAD/'ZI-E_v0_3_CUTAWAY.step'))
    try: cut.save(str(CAD/'ZI-E_v0_3_CUTAWAY.gltf'))
    except Exception as e: print('gltf cut warning',e)
    # printable parts
    for name,shape in printable_parts().items():
        exporters.export(shape,str(STL/f'{name}.stl'),tolerance=0.06,angularTolerance=0.12)
        try: exporters.export(shape,str(CAD/f'{name}.step'))
        except Exception as e: print('step part warning',name,e)
    # tool prototypes at origin
    for name,shape in [('gripper_tool_body',gripper_tool((0,0,0))['gripper_palm']),('magnet_tool_housing',magnet_tool((0,0,0))['magnet_housing'])]:
        exporters.export(shape,str(STL/f'{name}.stl'),tolerance=0.06,angularTolerance=0.12)
        exporters.export(shape,str(CAD/f'{name}.step'))
    # simple overall bboxes
    (DATA/'assembly_bounding_boxes.json').write_text(json.dumps({k:[round(x,2) for x in v] for k,v in bboxes.items()},indent=2),encoding='utf-8')
    print('built',ROOT)
    print('bboxes',bboxes)
    rows,total,com=mass_model(); print('mass g',total,'COM',com)

if __name__=='__main__': export_all()
