import random
import math

def generate_asteroid_belt(num_asteroids=200):
    xml_parts = []

    min_radius = 7.5
    max_radius = 9.5
    thickness = 1.0

    for i in range(num_asteroids):
        angle = random.uniform(0, 2 * math.pi)
        radius = random.uniform(min_radius, max_radius)
        vertical_offset = random.uniform(-thickness/2, thickness/2)

        orbit_speed = random.uniform(25, 35)

        size = min(0.005 + random.expovariate(5.0), 0.01)

        points = []
        for j in range(4):
            angle_offset = j * (math.pi / 2)
            final_angle = angle + angle_offset
            x = radius * math.cos(final_angle)
            z = radius * math.sin(final_angle)
            y = vertical_offset

            points.append((x, y, z))

        asteroid_xml = f"""
<group>
    <transform>
        <translate time="{orbit_speed:.1f}" align="true">
            <point x="{points[0][0]:.2f}" y="{points[0][1]:.2f}" z="{points[0][2]:.2f}"/>
            <point x="{points[1][0]:.2f}" y="{points[1][1]:.2f}" z="{points[1][2]:.2f}"/>
            <point x="{points[2][0]:.2f}" y="{points[2][1]:.2f}" z="{points[2][2]:.2f}"/>
            <point x="{points[3][0]:.2f}" y="{points[3][1]:.2f}" z="{points[3][2]:.2f}"/>
        </translate>
        <rotate x="{random.uniform(0, 1):.2f}" 
                y="{random.uniform(0, 1):.2f}" 
                z="{random.uniform(0, 1):.2f}" 
                time="{random.uniform(2, 10):.1f}"/>
        <scale x="{size:.3f}" y="{size:.3f}" z="{size:.3f}"/>
    </transform>
    <models>
        <model file="sphere.3d"/>
    </models>
</group>"""
        xml_parts.append(asteroid_xml)

    return '\n'.join(xml_parts)

asteroids_xml = generate_asteroid_belt(500)

with open('asteroid_belt_circular.xml', 'w') as f:
    f.write(f"""<!-- Asteroid Belt (200 objects with circular orbits) -->
<group>
{asteroids_xml}
</group>""")
