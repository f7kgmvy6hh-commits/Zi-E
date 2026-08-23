// ZI-E CAD v0.1 — parametric packaging / architecture model
// Units: mm
// Purpose: geometry, packing, swept-volume and proportions. NOT manufacturing-final.
// Design direction: compact desktop robot, old visual personality + current engineering.

$fn = 36;
mode = "exterior"; // exterior | cutaway | skeleton

// ---------- Global provisional envelopes derived from selected components ----------
base_w = 188;
base_d = 148;
base_h = 62;
body_w = 174;
body_d = 132;
body_h = 112;
body_z0 = 58;

head_w = 118;
head_h = 80;
head_d = 34;
head_lift_active = 36;
head_center_z = body_z0 + body_h + head_lift_active + head_h/2;

// Display candidate envelope, landscape orientation
screen_w = 84.52;
screen_h = 55.26;
screen_t = 4.0;

// Camera reservation (final FPC module target; larger prototype breakout does not define head)
camera_w = 26;
camera_h = 18;
camera_d = 16;

wheel_d = 62;
wheel_t = 19;
wheel_x = base_w/2 + wheel_t/2 - 7;
wheel_y = 8;
wheel_z = 31;

shoulder_x = body_w/2 + 20;
shoulder_z = body_z0 + body_h - 22;
upper_arm_len = 88;
forearm_len = 96;
forearm_extension_visual = 18; // active pose only; mechanism reserved for ~60mm-class stroke
hand_len = 52;
arm_radius = 18;

// Component envelopes
sts3215 = [45.2,24.7,35];
hl2915  = [34,20,23];
sc09     = [23.2,12,25.5];
battery_pack = [72,62,40]; // 3S2P 18650 cassette envelope incl. cell spacing/protection
main_pcb = [88,54,14];
safety_pcb = [70,44,12];
speaker = [40,28.5,12];

// ---------- Helpers ----------
module rounded_box(size=[10,10,10], r=2, center=true){
    sx=size[0]; sy=size[1]; sz=size[2];
    translate(center?[0,0,0]:[sx/2,sy/2,sz/2])
    minkowski(){
        cube([max(0.1,sx-2*r),max(0.1,sy-2*r),max(0.1,sz-2*r)], center=true);
        sphere(r=r);
    }
}

module capsule(p1=[0,0,0], p2=[0,0,10], r=5){
    hull(){ translate(p1) sphere(r=r); translate(p2) sphere(r=r); }
}

module comp_box(pos, size, c=[0.2,0.5,0.8,0.6], r=1){
    color(c) translate(pos) rounded_box(size,r,true);
}

module wheel(side=1){
    x=side*wheel_x;
    color([0.08,0.08,0.09]) translate([x,wheel_y,wheel_z]) rotate([0,90,0])
        cylinder(d=wheel_d,h=wheel_t,center=true);
    color([0.25,0.28,0.3]) translate([x,wheel_y,wheel_z]) rotate([0,90,0])
        cylinder(d=40,h=wheel_t+1,center=true);
}

module ball_caster(){
    color([0.18,0.18,0.2]) translate([0,-base_d/2+24,10]) sphere(d=20);
    color([0.4,0.4,0.42]) translate([0,-base_d/2+24,20]) cylinder(d=28,h=6,center=true);
}

module base_shell(alpha=1){
    color([0.78,0.8,0.82,alpha]) translate([0,0,base_h/2]) rounded_box([base_w,base_d,base_h],16,true);
    // lower darker service band
    color([0.12,0.14,0.16,alpha]) translate([0,10,18]) rounded_box([base_w-18,base_d-10,24],8,true);
}

module torso_shell(alpha=1){
    // main broad lower torso
    color([0.86,0.87,0.88,alpha]) translate([0,0,body_z0+body_h/2]) rounded_box([body_w,body_d,body_h],18,true);
    // subtle waist inset
    color([0.16,0.18,0.2,alpha]) translate([0,5,body_z0+18]) rounded_box([body_w-30,body_d-10,18],7,true);
    // seamless heart/RGB diffuser (dark when off)
    color([0.06,0.10,0.12,alpha]) translate([0,-body_d/2-1,body_z0+55]) rounded_box([30,2,8],3,true);
}

module head_shell(alpha=1){
    z=head_center_z;
    color([0.83,0.84,0.86,alpha]) translate([0,0,z]) rounded_box([head_w,head_d,head_h],10,true);
    // dark face recess
    color([0.035,0.045,0.055,alpha]) translate([0,-head_d/2-1,z-2]) rounded_box([head_w-14,3,head_h-12],7,true);
    // display glass
    color([0.02,0.03,0.04,alpha]) translate([0,-head_d/2-3,z-5]) cube([screen_w,screen_t,screen_h],center=true);
    // camera brow window
    color([0.03,0.04,0.05,alpha]) translate([0,-head_d/2-3,z+head_h/2-10]) rounded_box([38,3,7],2,true);
    // camera aperture
    color([0.08,0.12,0.14,alpha]) translate([0,-head_d/2-5,z+head_h/2-10]) cylinder(d=4.5,h=3,center=true, $fn=24);
}

module neck_mechanics(alpha=1){
    // lift rail and screw at rear of head bay
    color([0.35,0.38,0.4,alpha]) translate([0,22,body_z0+body_h+18]) cube([9,8,88],center=true);
    color([0.55,0.55,0.57,alpha]) translate([13,22,body_z0+body_h+18]) cylinder(d=5,h=88,center=true);
    // pan bearing ring
    color([0.18,0.2,0.22,alpha]) translate([0,0,body_z0+body_h+head_lift_active-4]) cylinder(d=52,h=10,center=true);
    // wide neck shroud
    color([0.12,0.14,0.16,alpha]) translate([0,0,body_z0+body_h+head_lift_active-3]) rounded_box([55,34,18],7,true);
}

module arm(side=1){
    sx=side*shoulder_x;
    // shoulder pod
    color([0.11,0.12,0.14]) translate([sx,0,shoulder_z]) sphere(d=52);
    color([0.72,0.74,0.76]) translate([sx,0,shoulder_z]) rotate([90,0,0]) cylinder(d=42,h=20,center=true);

    // downward natural pose, slightly outward
    p0=[sx,0,shoulder_z-5];
    p1=[side*(shoulder_x+24),-1,shoulder_z-upper_arm_len];
    p2=[side*(shoulder_x+20),-8,shoulder_z-upper_arm_len-forearm_len];

    color([0.75,0.77,0.79]) capsule(p0,p1,arm_radius);
    color([0.13,0.14,0.16]) translate(p1) sphere(d=40);
    color([0.78,0.8,0.82]) capsule(p1,p2,arm_radius-2);

    // telescopic inner forearm visual
    p3=[side*(shoulder_x+18),-10,p2[2]-forearm_extension_visual];
    color([0.34,0.36,0.38]) capsule(p2,p3,arm_radius-6);

    // wrist pitch yoke
    color([0.12,0.13,0.15]) translate(p3) sphere(d=34);

    // default gripper tool simplified
    palm=[p3[0],p3[1]-1,p3[2]-20];
    color([0.18,0.19,0.21]) translate(palm) rounded_box([34,28,30],5,true);
    color([0.12,0.13,0.15]) capsule([palm[0]-10,palm[1],palm[2]-8],[palm[0]-14,palm[1],palm[2]-hand_len],5);
    color([0.12,0.13,0.15]) capsule([palm[0]+10,palm[1],palm[2]-8],[palm[0]+14,palm[1],palm[2]-hand_len],5);
}

module exterior(){
    base_shell(1);
    torso_shell(1);
    neck_mechanics(1);
    head_shell(1);
    wheel(-1); wheel(1); ball_caster();
    arm(-1); arm(1);
    // front/rear anti-tip skid hints
    color([0.1,0.1,0.11]) translate([0,-base_d/2+6,4]) rounded_box([40,10,6],2,true);
    color([0.1,0.1,0.11]) translate([0, base_d/2-6,4]) rounded_box([40,10,6],2,true);
}

module internals(){
    // battery cassette low and central
    comp_box([0,12,35],battery_pack,[0.12,0.45,0.18,0.85],4);
    // rear service electronics
    comp_box([0,body_d/2-22,body_z0+72],main_pcb,[0.1,0.45,0.75,0.85],2);
    comp_box([0,body_d/2-22,body_z0+37],safety_pcb,[0.85,0.45,0.08,0.85],2);
    // speaker lower-front
    comp_box([-38,-body_d/2+20,body_z0+24],speaker,[0.35,0.12,0.5,0.85],2);
    // head components
    comp_box([0,-head_d/2+6,head_center_z-5],[screen_w,8,screen_h],[0.05,0.2,0.35,0.8],1);
    comp_box([0,-6,head_center_z+head_h/2-12],[camera_w,camera_d,camera_h],[0.15,0.65,0.2,0.9],2);
    // pan servo below head
    comp_box([-32,6,body_z0+body_h+head_lift_active-4],sc09,[0.8,0.25,0.1,0.9],2);
    // tilt servo in head rear side
    comp_box([head_w/2-22,7,head_center_z],sc09,[0.8,0.25,0.1,0.9],2);
    // lift motor
    comp_box([30,24,body_z0+body_h+3],[16,14,38],[0.8,0.25,0.1,0.9],2);
    // shoulder servos envelopes
    for(side=[-1,1]){
      comp_box([side*(body_w/2-7),8,shoulder_z+2],sts3215,[0.75,0.25,0.08,0.75],3);
      comp_box([side*(body_w/2-21),-2,shoulder_z-18],[24.7,45.2,35],[0.75,0.25,0.08,0.55],3);
    }
    // sensor markers: 4 downward + 4 horizontal ToF zones
    for(x=[-base_w/2+24,base_w/2-24], y=[-base_d/2+22,base_d/2-22]){
      color([0.15,0.8,0.8]) translate([x,y,8]) cube([8,8,4],center=true);
      color([0.15,0.8,0.8]) translate([x*0.96,y*0.92,28]) cube([8,4,8],center=true);
    }
}

module cutaway(){
    // translucent shell so packaging is readable
    base_shell(0.22);
    torso_shell(0.18);
    neck_mechanics(0.55);
    head_shell(0.16);
    wheel(-1); wheel(1); ball_caster();
    internals();
}

module skeleton(){
    // structural envelopes only
    color([0.25,0.28,0.3]) translate([0,10,35]) rounded_box([150,100,18],5,true); // lower chassis
    // shoulder towers
    for(side=[-1,1]) color([0.32,0.34,0.36]) translate([side*65,18,body_z0+75]) rounded_box([24,32,100],5,true);
    // rear bridge around head bay
    color([0.32,0.34,0.36]) translate([0,24,body_z0+body_h-8]) rounded_box([145,20,18],4,true);
    neck_mechanics(1);
    internals();
    wheel(-1); wheel(1); ball_caster();
}

if(mode=="exterior") exterior();
else if(mode=="cutaway") cutaway();
else skeleton();
