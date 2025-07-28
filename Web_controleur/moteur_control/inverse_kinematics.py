import math
import matplotlib.pyplot as plt

def inverse_kinematics_2d_flexible(x, y, d1, d2, theta_max_deg):
    r = math.hypot(x, y)

    max_reach = d1 + d2
    min_reach = abs(d1 - d2)

    # Conversion de l'angle maximum en radians
    theta_max = math.radians(theta_max_deg)

    # Clamp la distance si hors de portée
    if r > max_reach:
        x *= max_reach / r
        y *= max_reach / r
        r = max_reach
    elif r < min_reach:
        x *= min_reach / r
        y *= min_reach / r
        r = min_reach

    # Angle du genou (theta)
    cos_theta = (r ** 2 - d1 ** 2 - d2 ** 2) / (2 * d1 * d2)
    cos_theta = max(-1, min(1, cos_theta))
    theta = math.acos(cos_theta)

    # Limitation du genou
    if theta > theta_max:
        theta = theta_max  # on le bride

        # Nouvelle distance atteignable avec ce theta_max
        effective_r = math.sqrt(d1 ** 2 + d2 ** 2 + 2 * d1 * d2 * math.cos(theta))

        x *= effective_r / r
        y *= effective_r / r
        r = effective_r

    # Angle vers la cible
    alpha = math.atan2(y, x)

    # Calcul de beta
    cos_beta = (r ** 2 + d2 ** 2 - d1 ** 2) / (2 * r * d2)
    cos_beta = max(-1, min(1, cos_beta))
    beta = math.acos(cos_beta)

    # Angle du 1er segment
    phi = alpha - beta

    return phi, theta


def draw_leg(x_target, y_target, d1, d2):
    theta_max_deg = 150

    phi, theta = inverse_kinematics_2d_flexible(x_target, y_target, d1, d2, theta_max_deg)

    #phi = 0
    #theta = 0



    # Position genou
    x1 = d2 * math.cos(phi)
    y1 = d2 * math.sin(phi)

    # Position pied
    x2 = x1 + d1 * math.cos(phi + theta)
    y2 = y1 + d1 * math.sin(phi + theta)

    # Affichage des angles
    print(f"phi  = {phi:.3f} rad  ({math.degrees(phi):.2f}°)")
    print(f"theta = {theta:.3f} rad  ({math.degrees(theta):.2f}°)")

    # Tracé
    plt.figure(figsize=(6, 6))
    plt.plot([0, x1], [0, y1], 'ro-', label="Segment 1 (d2)")
    plt.plot([x1, x2], [y1, y2], 'bo-', label="Segment 2 (d1)")
    plt.plot(x_target, y_target, 'gx', markersize=10, label="Cible")
    plt.title(f"Patte articulée avec θ max = {theta_max_deg}°")
    plt.xlabel("x")
    plt.ylabel("y")
    plt.axis('equal')
    plt.grid(True)
    plt.legend()
    plt.xlim(-d1 - d2 - 1, d1 + d2 + 1)
    plt.ylim(-d1 - d2 - 1, d1 + d2 + 1)
    plt.show()


# Exemple d’utilisation :
draw_leg(x_target=0, y_target=1, d1=4, d2=4)