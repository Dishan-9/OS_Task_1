import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation, PillowWriter

# 1. Load the real C++ execution log
print("Reading execution_log.csv...")
df = pd.read_csv("execution_log.csv")

MATRIX_SIZE = 100
C_display = np.full((MATRIX_SIZE, MATRIX_SIZE), np.nan)

# 2. Setup Figure
fig, (ax_grid, ax_heat) = plt.subplots(1, 2, figsize=(11, 5))
fig.patch.set_facecolor('#181825')

# Left Panel: Live Fill
ax_grid.set_facecolor('#11111b')
ax_grid.set_title("Matrix C (Cells Populating Live)", color='white', fontsize=11, pad=10)
ax_grid.set_xticks([])
ax_grid.set_yticks([])

c_min, c_max = df['val'].min(), df['val'].max()
im_c = ax_grid.imshow(C_display, cmap='magma', vmin=c_min, vmax=c_max)

# Marker for current cell
marker = plt.Rectangle((-0.5, -0.5), 1, 1, fill=False, edgecolor='cyan', linewidth=2)
ax_grid.add_patch(marker)

# Right Panel: Progress
ax_heat.set_facecolor('#181825')
ax_heat.set_title("Completion Rate", color='white', fontsize=11, pad=10)
ax_heat.tick_params(colors='white')
ax_heat.set_xlim(0, len(df))
ax_heat.set_ylim(0, 100)
line, = ax_heat.plot([], [], color='#89b4fa', lw=2)

status_label = fig.text(0.5, 0.04, "", ha="center", color="white", fontsize=10, fontfamily="monospace")

# 3. Animation Settings (Chunked frames to keep the GIF smooth & small)
steps_per_frame = 100  # 10,000 cells / 100 = 100 frames
total_frames = (len(df) // steps_per_frame)
progress_x, progress_y = [], []

def animate(frame):
    start = frame * steps_per_frame
    end = min(start + steps_per_frame, len(df))
    
    last_r, last_c = 0, 0
    for i in range(start, end):
        r = int(df.iloc[i]['row'])
        c = int(df.iloc[i]['col'])
        v = df.iloc[i]['val']
        C_display[r, c] = v
        last_r, last_c = r, c

    im_c.set_data(C_display)
    marker.set_xy((last_c - 0.5, last_r - 0.5))
    
    progress_x.append(end)
    progress_y.append((end / len(df)) * 100)
    line.set_data(progress_x, progress_y)
    
    status_label.set_text(f"Processed: {end}/{len(df)} cells ({progress_y[-1]:.1f}%) | Active: C[{last_r}][{last_c}]")
    return [im_c, marker, line, status_label]

anim = FuncAnimation(fig, animate, frames=total_frames, interval=40, blit=False)

output_name = "matrix_mult_demo.gif"
print(f"Generating {output_name} (this will take ~10-15 seconds)...")
anim.save(output_name, writer=PillowWriter(fps=20))
print(f"Successfully generated {output_name}!")