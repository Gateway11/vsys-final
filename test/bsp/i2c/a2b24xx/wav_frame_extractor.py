#!/usr/bin/python3
"""
WAV Frame Extractor
A tool for analyzing WAV files, inspecting frame timestamps, and extracting audio segments
Usage: wav_frame_extractor <wav_file> [output_directory] [chunk_size]
"""

import struct
import os
import sys
import shutil

def parse_wav_header(filename, chunk_size=128):
    """
    Parse WAV file header and return frame related information
    """
    try:
        with open(filename, 'rb') as f:
            # Read RIFF header
            chunk_id = f.read(4)
            if chunk_id != b'RIFF':
                raise ValueError("Not a valid WAV file")
            
            chunk_size_total = struct.unpack('<I', f.read(4))[0]
            format = f.read(4)
            
            # Read fmt subchunk
            subchunk1_id = f.read(4)
            if subchunk1_id != b'fmt ':
                raise ValueError("fmt subchunk not found")
            
            subchunk1_size = struct.unpack('<I', f.read(4))[0]
            audio_format = struct.unpack('<H', f.read(2))[0]
            num_channels = struct.unpack('<H', f.read(2))[0]
            sample_rate = struct.unpack('<I', f.read(4))[0]
            byte_rate = struct.unpack('<I', f.read(4))[0]
            block_align = struct.unpack('<H', f.read(2))[0]
            bits_per_sample = struct.unpack('<H', f.read(2))[0]
            
            # Find data subchunk
            while True:
                subchunk2_id = f.read(4)
                if not subchunk2_id:
                    raise ValueError("data subchunk not found")
                
                if subchunk2_id == b'data':
                    break
                
                # Skip other subchunks
                subchunk2_size = struct.unpack('<I', f.read(4))[0]
                f.seek(subchunk2_size, 1)
            
            # Read data subchunk size
            subchunk2_size = struct.unpack('<I', f.read(4))[0]
            data_start_pos = f.tell()  # Record audio data start position
            
            # Calculate frame count considering chunk_size
            bytes_per_sample = bits_per_sample // 8
            bytes_per_frame = num_channels * bytes_per_sample
            bytes_per_chunk = chunk_size * bytes_per_frame
            
            # Total frames based on data size and chunk organization
            total_chunks = subchunk2_size // bytes_per_chunk
            remaining_bytes = subchunk2_size % bytes_per_chunk
            remaining_frames = remaining_bytes // bytes_per_frame
            
            total_frames = (total_chunks * chunk_size) + remaining_frames
            duration = total_frames / sample_rate
            
            print(f"File: {filename}")
            print(f"Frame count: {total_frames}")
            print(f"Sample rate: {sample_rate} Hz")
            print(f"Channels: {num_channels}")
            print(f"Bit depth: {bits_per_sample} bits")
            print(f"Duration: {duration:.2f} seconds")
            print(f"Data size: {subchunk2_size} bytes")
            
            return {
                'frames': total_frames,
                'sample_rate': sample_rate,
                'channels': num_channels,
                'bits_per_sample': bits_per_sample,
                'data_start_pos': data_start_pos,
                'data_size': subchunk2_size,
                'duration': duration,
                'bytes_per_sample': bytes_per_sample,
                'block_align': block_align,
                'chunk_size': chunk_size,
                'bytes_per_frame': bytes_per_frame,
                'bytes_per_chunk': bytes_per_chunk,
                'total_chunks': total_chunks,
                'remaining_frames': remaining_frames
            }
            
    except Exception as e:
        print(f"Error: {e}")
        return None

def load_timestamps(ts_filename):
    """
    Load timestamp file, one timestamp per line
    """
    if not os.path.exists(ts_filename):
        print(f"Timestamp file {ts_filename} does not exist")
        return None
    
    timestamps = []
    try:
        with open(ts_filename, 'r') as f:
            for line_num, line in enumerate(f, 1):
                line = line.strip()
                if line:  # Skip empty lines
                    try:
                        timestamp = int(line)
                        timestamps.append(timestamp)
                    except ValueError:
                        print(f"Warning: Line {line_num} is not a valid timestamp: {line}")
        
        print(f"Loaded {len(timestamps)} timestamps from {ts_filename}")
        return timestamps
        
    except Exception as e:
        print(f"Error reading timestamp file: {e}")
        return None

def save_wav_segment(input_file, output_file, wav_info, start_frame, end_frame=None):
    """
    Save a segment of the WAV file from start_frame to end_frame
    If end_frame is None, save from start_frame to the end
    """
    try:
        if end_frame is None:
            end_frame = wav_info['frames'] - 1
        
        if start_frame < 0 or start_frame >= wav_info['frames']:
            print(f"Error: Start frame {start_frame} is out of range")
            return False
        
        if end_frame < start_frame or end_frame >= wav_info['frames']:
            print(f"Error: End frame {end_frame} is out of range")
            return False
        
        # Calculate byte positions
        bytes_per_frame = wav_info['bytes_per_frame']
        start_byte = wav_info['data_start_pos'] + (start_frame * bytes_per_frame)
        end_byte = wav_info['data_start_pos'] + ((end_frame + 1) * bytes_per_frame)
        segment_size = end_byte - start_byte
        
        print(f"Extracting frames {start_frame} to {end_frame}")
        
        # Read the original header
        with open(input_file, 'rb') as infile:
            header_data = infile.read(wav_info['data_start_pos'])
            
            # Update the data size in the header
            header_bytearray = bytearray(header_data)
            
            # Find the position of the data chunk size field
            data_marker_pos = header_data.find(b'data')
            if data_marker_pos != -1:
                # Update the data size (little-endian)
                data_size_pos = data_marker_pos + 4
                header_bytearray[data_size_pos:data_size_pos+4] = struct.pack('<I', segment_size)
                
                # Update the overall file size (at position 4)
                file_size = len(header_bytearray) + segment_size - 8  # Subtract 8 for "RIFF" and size field
                header_bytearray[4:8] = struct.pack('<I', file_size)
            
            # Write the updated header to the output file
            with open(output_file, 'wb') as outfile:
                outfile.write(header_bytearray)
                
                # Seek to the start position in the input file
                infile.seek(start_byte)
                
                # Copy the audio data segment
                bytes_remaining = segment_size
                while bytes_remaining > 0:
                    chunk_size = min(8192, bytes_remaining)
                    data = infile.read(chunk_size)
                    if not data:
                        break
                    outfile.write(data)
                    bytes_remaining -= len(data)
        
        print(f"Successfully saved segment to: {output_file}")
        print(f"Segment duration: {(end_frame - start_frame + 1) / wav_info['sample_rate']:.2f} seconds")
        return True
        
    except Exception as e:
        print(f"Error saving WAV segment: {e}")
        return False

def select_frame_and_extract(wav_info, timestamps, input_file, output_dir):
    """
    Interactive frame selection and extract audio segments
    """
    total_frames = wav_info['frames']
    sample_rate = wav_info['sample_rate']
    
    if timestamps and len(timestamps) != total_frames:
        print(f"Warning: Timestamp count ({len(timestamps)}) does not match frame count ({total_frames})!")
    
    print(f"\n=== Frame Selection and Extraction Mode ===")
    print(f"Total frames: {total_frames}")
    print(f"Duration: {wav_info['duration']:.2f} seconds")
    print(f"Output directory: {output_dir}")
    print("Commands:")
    print("  [frame_number] - Select a frame and show timestamp")
    print("  s [start_frame] [end_frame] - Save segment from start_frame to end_frame")
    print("  sf [start_frame] - Save from start_frame to end of file")
    print("  q - Exit")
    
    while True:
        try:
            user_input = input("\nEnter command: ").strip()
            
            if user_input.lower() == 'q':
                break
            
            parts = user_input.split()
            if len(parts) == 1:
                # Single frame selection
                frame_num = int(parts[0])
                
                if frame_num < 0 or frame_num >= total_frames:
                    print(f"Error: Frame number must be between 0 and {total_frames - 1}")
                    continue
                
                # Display timestamp
                if timestamps and frame_num < len(timestamps):
                    timestamp = timestamps[frame_num]
                    time_in_seconds = frame_num / sample_rate
                    
                    print(f"Frame number: {frame_num}")
                    print(f"Timestamp: {timestamp}")
                    print(f"Time: {time_in_seconds:.6f} seconds")
                else:
                    time_in_seconds = frame_num / sample_rate
                    print(f"Frame number: {frame_num}")
                    print(f"Time: {time_in_seconds:.6f} seconds")
            
            elif len(parts) >= 2 and parts[0].lower() == 's':
                # Save segment
                start_frame = int(parts[1])
                
                if len(parts) >= 3:
                    end_frame = int(parts[2])
                else:
                    end_frame = None
                
                # Generate output filename based on timestamp
                base_name = os.path.splitext(os.path.basename(input_file))[0]
                
                if timestamps and start_frame < len(timestamps):
                    # Use timestamp for filename
                    start_timestamp = timestamps[start_frame]
                    
                    if end_frame is not None and end_frame < len(timestamps):
                        end_timestamp = timestamps[end_frame]
                        output_file = os.path.join(output_dir, f"{base_name}_timestamp_{start_timestamp}_to_{end_timestamp}.wav")
                    else:
                        output_file = os.path.join(output_dir, f"{base_name}_timestamp_{start_timestamp}.wav")
                else:
                    # Fallback to frame numbers if no timestamps
                    if end_frame is not None:
                        output_file = os.path.join(output_dir, f"{base_name}_frame_{start_frame}_to_{end_frame}.wav")
                    else:
                        output_file = os.path.join(output_dir, f"{base_name}_frame_{start_frame}.wav")
                
                # Save the segment
                if save_wav_segment(input_file, output_file, wav_info, start_frame, end_frame):
                    print(f"Segment saved to: {output_file}")
                else:
                    print("Failed to save segment")
            
            elif len(parts) >= 2 and parts[0].lower() == 'sf':
                # Save from frame to end
                start_frame = int(parts[1])
                
                # Generate output filename based on timestamp
                base_name = os.path.splitext(os.path.basename(input_file))[0]
                
                if timestamps and start_frame < len(timestamps):
                    # Use timestamp for filename
                    start_timestamp = timestamps[start_frame]
                    output_file = os.path.join(output_dir, f"{base_name}_timestamp_{start_timestamp}.wav")
                else:
                    # Fallback to frame number if no timestamps
                    output_file = os.path.join(output_dir, f"{base_name}_frame_{start_frame}.wav")
                
                # Save the segment
                if save_wav_segment(input_file, output_file, wav_info, start_frame):
                    print(f"Segment saved to: {output_file}")
                else:
                    print("Failed to save segment")
            
            else:
                print("Invalid command. Use:")
                print("  [frame_number] - Select frame")
                print("  s [start] [end] - Save segment")
                print("  sf [start] - Save from start to end")
                print("  q - Quit")
                
        except ValueError:
            print("Error: Please enter valid numbers")
        except KeyboardInterrupt:
            print("\nProgram interrupted by user")
            break

def main():
    # Get filename from command line arguments
    if len(sys.argv) < 2:
        print("Usage: python wav_frame_extractor.py <wav_file> [output_directory] [chunk_size]")
        print("Example: python wav_frame_extractor.py record.wav ./extracted_segments 128")
        return
    
    # File paths
    wav_filename = sys.argv[1]
    
    # Output directory
    if len(sys.argv) >= 3:
        output_dir = sys.argv[2]
    else:
        output_dir = "./extracted_segments"
    
    # Chunk size (default 128 for arecord)
    chunk_size = 128
    if len(sys.argv) >= 4:
        try:
            chunk_size = int(sys.argv[3])
        except ValueError:
            print("Error: Chunk size must be an integer")
            return
    
    # Create output directory if it doesn't exist
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
        print(f"Created output directory: {output_dir}")
    
    ts_filename = wav_filename + ".ts"
    
    # 1. Parse WAV file
    print("Parsing WAV file...")
    wav_info = parse_wav_header(wav_filename, chunk_size)
    if not wav_info:
        return
    
    print("-" * 50)
    
    # 2. Load timestamp file
    print("Loading timestamp file...")
    timestamps = load_timestamps(ts_filename)
    
    print("-" * 50)
    
    # 3. Interactive frame selection and extraction
    select_frame_and_extract(wav_info, timestamps, wav_filename, output_dir)

if __name__ == "__main__":
    main()
