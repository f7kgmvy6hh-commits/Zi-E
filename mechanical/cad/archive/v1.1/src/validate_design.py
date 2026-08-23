#!/usr/bin/env python3
from __future__ import annotations
import json, math, csv
from pathlib import Path
import numpy as np
from zie_cad import *

ROOT=Path(__file__).resolve().parents[1]; DATA=ROOT/'data'; DOCS=ROOT/'docs'

def seg_dist(p,a,b):
    p=np.array(p,float); a=np.array(a,float); b=np.array(b,float); v=b-a
    t=np.dot(p-a,v)/np.dot(v,v); t=max(0,min(1,float(t))); q=a+t*v
    return float(np.linalg.norm(p-q))

def support_margins(comxy,wheel_y=None):
    wy=P['wheel_y'] if wheel_y is None else wheel_y
    tri=[(-P['wheel_x_active'],wy),(P['wheel_x_active'],wy),(0,P['caster_y'])]
    return [seg_dist(comxy,tri[i],tri[(i+1)%3]) for i in range(3)], tri

def calc_com(rows):
    total=sum(r['mass_g'] for r in rows)
    x=sum(r['mass_g']*r['x_mm'] for r in rows)/total
    y=sum(r['mass_g']*r['y_mm'] for r in rows)/total
    z=sum(r['mass_g']*r['z_mm'] for r in rows)/total
    return total,(x,y,z)

def main():
    tests=[]
    def T(name,value,limit,passed,notes=''):
        tests.append({'test':name,'value':value,'limit_or_target':limit,'pass':bool(passed),'notes':notes})

    # Packaging / clearances
    T('Head bay side clearance per side',(P['head_bay_w']-P['head_w'])/2,'>= 1.5 mm', (P['head_bay_w']-P['head_w'])/2>=1.5,'Docking only at centered pan/defined tilt.')
    T('Head bay depth clearance per side',(P['head_bay_d']-P['head_d'])/2,'>= 2.0 mm',(P['head_bay_d']-P['head_d'])/2>=2.0,'Allows shell tolerance and cable-safe docking.')
    T('Display side bezel each side',(P['head_w']-P['display_w'])/2,'>= 10 mm',(P['head_w']-P['display_w'])/2>=10,'Provides structure around glass and room for tilt frame.')
    T('Display vertical margin total',P['head_h']-P['display_h'],'>= 20 mm',P['head_h']-P['display_h']>=20,'Shared with top camera/mic brow.')
    T('Belly prototype PCB inside reserved keepout width',P['belly_matrix_keepout_w']-P['belly_matrix_proto_w'],'>= 20 mm spare',P['belly_matrix_keepout_w']-P['belly_matrix_proto_w']>=20,'Allows custom wider matrix later without changing torso envelope.')
    T('Belly prototype PCB inside reserved keepout height',P['belly_matrix_keepout_h']-P['belly_matrix_proto_h'],'>= 10 mm spare',P['belly_matrix_keepout_h']-P['belly_matrix_proto_h']>=10,'Allows baffle, connector, and optical tuning space.')
    T('Belly optical window thickness',P['belly_window_t'],'0.8-1.5 mm prototype range',0.8<=P['belly_window_t']<=1.5,'Final transmission/color requires optical coupon test, not CAD assumption.')
    # Sleep visible screen fraction above body top
    body_top=P['body_z0']+P['torso_h']; screen_center=P['head_sleep_bottom']+34
    screen_low=screen_center-P['display_active_h']/2; screen_high=screen_center+P['display_active_h']/2
    visible=max(0,screen_high-max(screen_low,body_top)); frac=visible/P['display_active_h']
    T('Sleep visible active-screen fraction',round(frac,3),'>= 0.75',frac>=0.75,'Neck hidden while most of face remains visible.')
    T('Shutdown head top flush',P['head_shutdown_bottom']+P['head_h']-body_top,'<= 1 mm above body top',P['head_shutdown_bottom']+P['head_h']-body_top<=1.0,'Protected shutdown has no screen/camera exposure above top plane.')

    # State bounding boxes
    bboxes={}
    for st in ('ACTIVE','SLEEP','SHUTDOWN'):
        a=build_assembly(st,False); bb=bbox_of_assy(a); bboxes[st]=bb
        T(f'{st} minimum Z',round(bb[2],3),'>= -0.1 mm',bb[2]>=-0.1,'No geometry should penetrate the desk plane.')
    T('Shutdown overall height',round(bboxes['SHUTDOWN'][5]-bboxes['SHUTDOWN'][2],1),'~182 mm',bboxes['SHUTDOWN'][5]<=183.0,'Compact pod target.')
    T('Active overall height',round(bboxes['ACTIVE'][5]-bboxes['ACTIVE'][2],1),'<= 290 mm',bboxes['ACTIVE'][5]<=290.0,'Desktop scale.')

    # Arm collision states vs cosmetic torso shell
    torso=torso_shell()
    for pose in ('active','stow','reach','desk'):
        totalv=0.0
        for side in (-1,1):
            am=arm_mechanism(side,pose,0,'gripper')
            for k in ['forearm','inner','wrist','quick','gripper_palm','finger_-1_prox','finger_-1_dist','finger_1_prox','finger_1_dist']:
                totalv += am[k].intersect(torso).Volume()
        T(f'Arm distal collision with torso: {pose}',round(totalv,2),'0 mm^3',totalv<1.0,'Shoulder mount overlap intentionally excluded.')

    # Wheel states
    base=base_shell()
    for state in ('active','stowed'):
        tiremin=999
        tirevol=0
        for side in (-1,1):
            wm=wheel_module(side,state); bb=wm['tire'].BoundingBox(); tiremin=min(tiremin,bb.zmin); tirevol+=wm['tire'].intersect(base).Volume()
        if state=='active':
            T('Active tire desk contact',round(tiremin,2),'~0 mm',abs(tiremin)<=0.2,'Powered wheels define two primary contact points.')
        else:
            T('Stowed wheel ground clearance',round(tiremin,2),'>= 4 mm',tiremin>=4,'Robot rests on four shutdown feet, not folded wheels.')
        T(f'Wheel tire/base collision {state}',round(tirevol,3),'< 2 mm^3 numerical',tirevol<2,'Near-zero boolean sliver is acceptable; no structural collision.')

    # Head collision with torso shell along docking states
    for st,hb in [('ACTIVE',P['head_active_bottom']),('SLEEP',P['head_sleep_bottom']),('SHUTDOWN',P['head_shutdown_bottom'])]:
        v=head_shell_shape(hb).intersect(torso).Volume()
        T(f'Head shell/torso collision {st}',round(v,3),'0 mm^3',v<1,'Head bay clears the cosmetic shell at docking pan/tilt.')

    # Motion timing / basic actuator numbers
    lift_speed=200*2/60 # mm/s
    T('Head lift nominal linear speed',round(lift_speed,2),'~6.7 mm/s',5<=lift_speed<=10,'T5x2 at 200 RPM; full protected travel ~15 s.')
    T('Head protected travel time',round(P['head_lift_stroke']/lift_speed,1),'<= 18 s',P['head_lift_stroke']/lift_speed<=18,'Can switch to faster N20 if user testing wants faster motion.')
    wheel_speed=75*math.pi*(P['wheel_d']/1000)/60
    T('Rated drive speed',round(wheel_speed,3),'0.20-0.30 m/s',0.20<=wheel_speed<=0.30,'Software speed is lower near edges and during manipulation.')

    # Torque sanity checks (rated, not stall)
    head_mass_kg=0.220; com_off_mm=20
    req_kgcm=head_mass_kg*(com_off_mm/10)
    avail_kgcm=0.7*2.0*0.85 # SC09 rated x 2:1 x assumed belt efficiency
    T('Head tilt rated torque margin',round(avail_kgcm/req_kgcm,2),'>= 2.0x',avail_kgcm/req_kgcm>=2,'Assumes head COM within 20 mm of tilt axis; measure real head before freeze.')
    wrist_req=0.150*4.0 # kg*cm, 150 g at 40 mm
    wrist_av=0.7*2*0.85
    T('Wrist pitch rated torque margin',round(wrist_av/wrist_req,2),'>= 1.7x',wrist_av/wrist_req>=1.7,'150 g tool at 40 mm; heavier tools trigger ST3036/HL2915 backup.')
    shoulder_req=0.26*10 + 0.25*18 # rough distal arm mass + 250g payload moment kgcm
    shoulder_av=10*1.4*0.85
    T('Shoulder rated torque sanity margin',round(shoulder_av/shoulder_req,2),'>= 1.5x',shoulder_av/shoulder_req>=1.5,'Approximation only; dummy-arm thermal/load test remains mandatory.')
    elbow_req=0.20*7 + 0.25*12
    elbow_av=4.5*2*0.85
    T('Elbow rated torque sanity margin',round(elbow_av/elbow_req,2),'>= 1.5x',elbow_av/elbow_req>=1.5,'Approximation only; uses planned ~2:1 belt reduction.')

    # Mass/COM and stability
    rows,total,com=mass_model()
    margins,tri=support_margins(com[:2]); minmargin=min(margins)
    T('Estimated total mass',total,'informational',True,'Current estimate before measured printed/component mass.')
    T('Baseline static support margin',round(minmargin,1),'>= 18 mm',minmargin>=18,'Minimum distance of COM projection to primary support triangle edge.')
    # one arm forward with 250g payload
    scen=[r.copy() for r in rows]
    for r in scen:
        if r['item']=='left arm complete': r['y_mm']=-65
    scen.append(dict(item='left payload',mass_g=250,x_mm=-96,y_mm=-150,z_mm=75,confidence='scenario',source='stability load case'))
    t1,c1=calc_com(scen); m1,_=support_margins(c1[:2]);
    T('One-arm 250g forward static margin',round(min(m1),1),'>= 15 mm',min(m1)>=15,'Allowed only at reduced base speed/acceleration.')
    # both arms forward payloads - expected restricted case
    scen2=[r.copy() for r in rows]
    for r in scen2:
        if 'arm complete' in r['item']: r['y_mm']=-65
    scen2 += [dict(item='left payload',mass_g=250,x_mm=-96,y_mm=-150,z_mm=75,confidence='scenario',source='stability load case'),dict(item='right payload',mass_g=250,x_mm=96,y_mm=-150,z_mm=75,confidence='scenario',source='stability load case')]
    t2,c2=calc_com(scen2); m2,_=support_margins(c2[:2])
    T('Two-arm 2x250g forward static margin',round(min(m2),1),'>= 15 mm for normal driving',min(m2)>=15,'Expected restricted case: base lock/very-low acceleration or smaller payload.')

    # Power sanity
    arm_branch_limit=5.0; wheel_stall=2*1.2; head_worst=2*1.0+1.1
    T('Protected worst commanded rail budget',round(2*arm_branch_limit+wheel_stall+head_worst,1),'< 20 A cell ceiling',2*arm_branch_limit+wheel_stall+head_worst<20,'Independent eFuses/current limits prevent theoretical all-joint stall from reaching cell ceiling.')

    # Output CSV/JSON/MD
    with open(DATA/'validation_matrix.csv','w',newline='',encoding='utf-8') as f:
        w=csv.DictWriter(f,fieldnames=tests[0].keys()); w.writeheader(); w.writerows(tests)
    summary={'tests_total':len(tests),'passed':sum(t['pass'] for t in tests),'failed':sum(not t['pass'] for t in tests),
             'estimated_mass_g':total,'estimated_com_mm':{'x':round(com[0],1),'y':round(com[1],1),'z':round(com[2],1)},
             'support_triangle_mm':tri,'state_bboxes_mm':{k:[round(x,2) for x in v] for k,v in bboxes.items()},
             'tests':tests}
    (DATA/'validation_summary.json').write_text(json.dumps(summary,indent=2),encoding='utf-8')
    lines=['# ZI-E CAD v1.1 — Automated Geometry / Engineering Validation','',f"**Result:** {summary['passed']}/{summary['tests_total']} checks pass. Failed checks are design restrictions or freeze gates, not silently ignored.",'',f"Estimated mass: **{total/1000:.2f} kg**; active estimated COM: **({com[0]:.1f}, {com[1]:.1f}, {com[2]:.1f}) mm**.",'','| Check | Value | Target | Result |','|---|---:|---|---|']
    for t in tests:
        lines.append(f"| {t['test']} | {t['value']} | {t['limit_or_target']} | {'PASS' if t['pass'] else 'RESTRICT / VERIFY'} |")
    lines += ['','## Important interpretation','',
              '- The two-arm 2×250 g forward case is intentionally allowed to fail the normal-driving margin. Firmware must treat that combination as a restricted manipulation state (base stationary or much lower payload/acceleration).',
              '- Torque calculations use **rated torque**, never stall torque, and include a simple belt-loss factor. Physical thermal/load tests remain the authority.',
              '- Mass values tagged low/medium confidence must be replaced with measured values after parts are printed/bought.',
              '- Collision tests cover the defined key poses. Continuous trajectory collision checking belongs in the simulation/firmware phase.']
    (DOCS/'VALIDATION_REPORT.md').write_text('\n'.join(lines),encoding='utf-8')
    print(json.dumps(summary,indent=2))

if __name__=='__main__': main()
